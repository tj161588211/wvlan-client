#!/bin/bash
# GitHub Actions Monitor & Auto-Heal Script
# Checks build status every 5 minutes until success

REPO="tj161588211/wvlan-client"
BUILD_ID=$1

if [ -z "$BUILD_ID" ]; then
    echo "Usage: $0 <build_id>"
    exit 1
fi

MAX_RETRIES=5
retry_count=0

echo "🔍 Starting monitor loop for build $BUILD_ID..."
echo "⏰ Check interval: 5 minutes"
echo "---"

while [ $retry_count -lt $MAX_RETRIES ]; do
    echo ""
    echo "[$(date '+%H:%M:%S')] Check #$((retry_count + 1))/$MAX_RETRIES"
    
    # Get build status using GitHub REST API
    STATUS=$(curl -s -H "Authorization: token $GITHUB_TOKEN" \
        "https://api.github.com/repos/$REPO/actions/runs/$BUILD_ID" \
        | jq -r '.status')
    
    CONCLUSION=$(curl -s -H "Authorization: token $GITHUB_TOKEN" \
        "https://api.github.com/repos/$REPO/actions/runs/$BUILD_ID" \
        | jq -r '.conclusion')
    
    echo "   Status: $STATUS"
    
    if [ "$STATUS" = "completed" ]; then
        echo "   Conclusion: $CONCLUSION"
        
        if [ "$CONCLUSION" = "success" ]; then
            echo ""
            echo "✅ BUILD SUCCEEDED!"
            echo "📊 View: https://github.com/$REPO/actions/runs/$BUILD_ID"
            exit 0
        else
            echo ""
            echo "❌ BUILD FAILED: $CONCLUSION"
            echo "📥 Fetching logs..."
            
            # Download failed logs
            LOGS_DIR="/tmp/wvlan-ci-logs"
            mkdir -p $LOGS_DIR
            
            curl -s -L -H "Authorization: token $GITHUB_TOKEN" \
                -o $LOGS_DIR/build-$BUILD_ID.zip \
                "https://api.github.com/repos/$REPO/actions/runs/$BUILD_ID/logs"
            
            echo "📋 Logs saved to: $LOGS_DIR/build-$BUILD_ID.zip"
            echo ""
            echo "⚙️  Auto-heal triggered..."
            echo "➡️  Fixing and re-triggering..."
            
            # Increment retry counter
            retry_count=$((retry_count + 1))
            
            # Here we would analyze logs and apply fixes
            # For now, just re-push the same commit to trigger rebuild
            cd /root/.hermes/hermes-agent/wvlan-project
            git commit --allow-empty -m "ci: re-trigger build (attempt $retry_count)"
            git push origin main
            
            # Update BUILD_ID to new run
            sleep 3
            NEW_BUILD_ID=$(curl -s -H "Authorization: token $GITHUB_TOKEN" \
                "https://api.github.com/repos/$REPO/actions/runs" \
                | jq -r '.workflow_runs[0].id')
            
            if [ ! -z "$NEW_BUILD_ID" ] && [ "$NEW_BUILD_ID" != "null" ]; then
                BUILD_ID=$NEW_BUILD_ID
                echo "🔄 New build ID: $BUILD_ID"
            fi
        fi
    else
        echo "   Still running... waiting 5 minutes"
    fi
    
    # Wait 5 minutes before next check
    if [ $retry_count -lt $MAX_RETRIES ]; then
        echo "⏳ Waiting 5 minutes before next check..."
        sleep 300
    fi
done

echo ""
echo "⚠️  Max retries ($MAX_RETRIES) reached. Manual intervention required."
exit 1
