-- +migrate Up
-- 创建连接日志表

CREATE TYPE connection_action AS ENUM ('connect', 'disconnect', 'reconnect');
CREATE TYPE connection_status AS ENUM ('success', 'failed');

CREATE TABLE IF NOT EXISTS connection_logs (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    user_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    device_id UUID NOT NULL REFERENCES devices(id) ON DELETE CASCADE,
    connection_id VARCHAR(255),
    action connection_action NOT NULL,
    status connection_status NOT NULL,
    client_ip VARCHAR(45),
    user_agent TEXT,
    error_msg TEXT,
    duration BIGINT,
    connection_time TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

-- 添加索引
CREATE INDEX idx_connection_logs_user_id ON connection_logs(user_id);
CREATE INDEX idx_connection_logs_device_id ON connection_logs(device_id);
CREATE INDEX idx_connection_logs_connection_id ON connection_logs(connection_id);

-- 添加注释
COMMENT ON TABLE connection_logs IS '设备连接日志表';
COMMENT ON COLUMN connection_logs.id IS '日志 ID，UUID 格式';
COMMENT ON COLUMN connection_logs.user_id IS '所属用户 ID';
COMMENT ON COLUMN connection_logs.device_id IS '设备 ID';
COMMENT ON COLUMN connection_logs.connection_id IS '连接 ID';
COMMENT ON COLUMN connection_logs.action IS '操作类型：connect, disconnect, reconnect';
COMMENT ON COLUMN connection_logs.status IS '连接状态：success, failed';
COMMENT ON COLUMN connection_logs.client_ip IS '客户端 IP';
COMMENT ON COLUMN connection_logs.user_agent IS '用户代理';
COMMENT ON COLUMN connection_logs.error_msg IS '错误信息';
COMMENT ON COLUMN connection_logs.duration IS '连接持续时间（毫秒）';

-- +migrate Down
DROP TABLE IF EXISTS connection_logs;
DROP TYPE IF EXISTS connection_status;
DROP TYPE IF EXISTS connection_action;
