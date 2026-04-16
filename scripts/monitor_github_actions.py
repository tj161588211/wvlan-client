#!/usr/bin/env python3
"""
GitHub Actions Monitor & Auto-Heal
Checks build status every 5 minutes, diagnoses failures, applies fixes, and re-triggers until success.
"""

import os
import sys
import time
import json
import subprocess
import requests
from pathlib import Path
from datetime import datetime

REPO = "tj161588211/wvlan-client"
GITHUB_TOKEN = os.getenv("GITHUB_TOKEN")
if not GITHUB_TOKEN:
    print("❌ GITHUB_TOKEN environment variable is required")
    sys.exit(1)

API_BASE = f"https://api.github.com/repos/{REPO}"
HEADERS = {"Authorization": f"token {GITHUB_TOKEN}"}

def get_build_status(build_id):
    """Get current status of a build."""
    resp = requests.get(f"{API_BASE}/actions/runs/{build_id}", headers=HEADERS)
    data = resp.json()
    return data.get("status"), data.get("conclusion"), data.get("head_sha")

def download_logs(build_id, output_dir):
    """Download build logs as zip."""
    os.makedirs(output_dir, exist_ok=True)
    resp = requests.get(f"{API_BASE}/actions/runs/{build_id}/logs", headers=HEADERS, stream=True)
    zip_path = Path(output_dir) / f"build-{build_id}.zip"
    with open(zip_path, "wb") as f:
        for chunk in resp.iter_content(chunk_size=8192):
            f.write(chunk)
    return zip_path

def analyze_failure(logs_dir):
    """Analyze failed logs and return error summary."""
    # Simple pattern matching on downloaded logs
    for log_file in Path(logs_dir).glob("*.log"):
        content = log_file.read_text(errors="ignore").lower()
        if "missing go.sum entry" in content:
            return "Go module: missing go.sum entries"
        if "vcpkg" in content and ("error" in content or "failed" in content):
            return "vcpkg installation failed"
        if "package.json not found" in content:
            return "Node.js dependencies missing"
        if "dotnet" in content and "error" in content:
            return ".NET build error"
    return "Unknown build error"

def apply_fixs(error_summary):
    """Apply fixes based on error analysis."""
    print(f"🔧 Analyzed error: {error_summary}")
    
    if "go.sum" in error_summary.lower():
        print("   → Go modules issue detected (should be fixed already)")
        return True
    
    # Add more fix logic here as needed
    print("   → No automatic fix implemented yet")
    return False

def trigger_new_build():
    """Push an empty commit to trigger new build."""
    repo_path = "/root/.hermes/hermes-agent/wvlan-project"
    try:
        subprocess.run(["git", "-C", repo_path, "commit", "--allow-empty", 
                       "-m", f"ci: re-trigger build at {datetime.now().isoformat()}"], check=True)
        result = subprocess.run(["git", "-C", repo_path, "push", "origin", "main"], 
                               capture_output=True, text=True, check=True)
        
        # Get new build ID
        time.sleep(2)
        resp = requests.get(f"{API_BASE}/actions/runs", headers=HEADERS)
        latest_run = resp.json()["workflow_runs"][0]
        return latest_run["id"], latest_run["url"]
    except Exception as e:
        print(f"❌ Failed to trigger new build: {e}")
        return None, None

def monitor_loop(build_id, max_retries=5):
    """Main monitoring loop - checks every 5 minutes."""
    retry_count = 0
    
    print(f"🔍 Starting monitor loop for build #{build_id}")
    print(f"⏰ Check interval: 5 minutes")
    print(f"🔄 Max retries: {max_retries}")
    print("-" * 60)
    
    while retry_count < max_retries:
        attempt = retry_count + 1
        print(f"\n[{datetime.now().strftime('%H:%M:%S')}] Check #{attempt}/{max_retries}")
        
        status, conclusion, sha = get_build_status(build_id)
        print(f"   Status: {status}")
        
        if status == "completed":
            print(f"   Conclusion: {conclusion}")
            
            if conclusion == "success":
                print("\n✅ BUILD SUCCEEDED!")
                print(f"📊 View: https://github.com/{REPO}/actions/runs/{build_id}")
                return True
            
            else:
                print(f"\n❌ BUILD FAILED: {conclusion}")
                print("📥 Fetching logs...")
                
                logs_dir = f"/tmp/wvlan-ci-logs-{build_id}"
                zip_path = download_logs(build_id, logs_dir)
                print(f"📋 Logs saved: {zip_path}")
                
                print("🔍 Analyzing failure...")
                error_summary = analyze_failure(logs_dir)
                
                print("⚙️  Applying fixes...")
                apply_fixs(error_summary)
                
                retry_count += 1
                
                if retry_count < max_retries:
                    print("🔄 Re-triggering build...")
                    new_build_id, build_url = trigger_new_build()
                    
                    if new_build_id:
                        build_id = new_build_id
                        print(f"   New build ID: #{build_id}")
                        print(f"   View: {build_url}")
                else:
                    print(f"\n⚠️  Max retries ({max_retries}) reached")
                    return False
        
        else:
            print("   Still running... ⏳ waiting 5 minutes")
        
        # Wait 5 minutes before next check (unless max retries reached)
        if retry_count < max_retries:
            print("⏰ Next check in 5 minutes...")
            for remaining in range(300, 0, -10):
                print(f"\r   {remaining // 60}:{'0' if remaining % 60 < 10 else ''}{remaining % 60} ", end="", flush=True)
                time.sleep(10)
            print()
    
    return False

def main():
    if len(sys.argv) < 2:
        print("Usage: python monitor-github-actions.py <build_id>")
        sys.exit(1)
    
    build_id = sys.argv[1]
    success = monitor_loop(build_id)
    sys.exit(0 if success else 1)

if __name__ == "__main__":
    main()
