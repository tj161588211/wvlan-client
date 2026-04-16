-- +migrate Up
-- 创建设备表

CREATE TYPE device_status AS ENUM ('online', 'offline');

CREATE TABLE IF NOT EXISTS devices (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    user_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    name VARCHAR(255) NOT NULL,
    type VARCHAR(50),
    os_type VARCHAR(100),
    version VARCHAR(50),
    ip VARCHAR(45),
    mac VARCHAR(17) NOT NULL UNIQUE,
    status device_status DEFAULT 'offline',
    last_seen TIMESTAMP WITH TIME ZONE,
    public_key TEXT,
    registered_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

-- 添加索引
CREATE INDEX idx_devices_user_id ON devices(user_id);
CREATE INDEX idx_devices_ip ON devices(ip);
CREATE INDEX idx_devices_mac ON devices(mac);

-- 添加注释
COMMENT ON TABLE devices IS '用户设备信息表';
COMMENT ON COLUMN devices.id IS '设备 ID，UUID 格式';
COMMENT ON COLUMN devices.user_id IS '所属用户 ID';
COMMENT ON COLUMN devices.name IS '设备名称';
COMMENT ON COLUMN devices.type IS '设备类型：windows, linux, macos, other';
COMMENT ON COLUMN devices.os_type IS '操作系统类型';
COMMENT ON COLUMN devices.version IS '操作系统版本';
COMMENT ON COLUMN devices.ip IS '当前 IP 地址';
COMMENT ON COLUMN devices.mac IS 'MAC 地址，唯一';
COMMENT ON COLUMN devices.status IS '设备状态：online, offline';
COMMENT ON COLUMN devices.last_seen IS '最后在线时间';
COMMENT ON COLUMN devices.public_key IS '设备公钥，用于安全通信';

-- +migrate Down
DROP TABLE IF EXISTS devices;
DROP TYPE IF EXISTS device_status;
