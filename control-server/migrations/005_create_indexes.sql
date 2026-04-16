-- +migrate Up
-- 创建额外索引和优化

-- 创建更新时间自动更新的触发器函数
CREATE OR REPLACE FUNCTION update_updated_at_column()
RETURNS TRIGGER AS $$
BEGIN
    NEW.updated_at = NOW();
    RETURN NEW;
END;
$$ language 'plpgsql';

-- 为需要更新时间的表添加触发器
CREATE TRIGGER update_users_updated_at BEFORE UPDATE ON users
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

CREATE TRIGGER update_devices_updated_at BEFORE UPDATE ON devices
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

CREATE TRIGGER update_configs_updated_at BEFORE UPDATE ON configs
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

-- 创建复合索引以提高查询性能
CREATE INDEX idx_devices_user_status ON devices(user_id, status);
CREATE INDEX idx_connection_logs_time ON connection_logs(connection_time DESC);
CREATE INDEX idx_configs_active ON configs(device_id, is_active) WHERE is_active = true;

-- +migrate Down
DROP TRIGGER IF EXISTS update_configs_updated_at ON configs;
DROP TRIGGER IF EXISTS update_devices_updated_at ON devices;
DROP TRIGGER IF EXISTS update_users_updated_at ON users;
DROP FUNCTION IF EXISTS update_updated_at_column();
DROP INDEX IF EXISTS idx_configs_active;
DROP INDEX IF EXISTS idx_connection_logs_time;
DROP INDEX IF EXISTS idx_devices_user_status;
