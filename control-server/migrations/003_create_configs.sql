-- +migrate Up
-- 创建配置表

CREATE TYPE config_type AS ENUM ('network', 'security', 'general');

CREATE TABLE IF NOT EXISTS configs (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    user_id UUID NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    device_id UUID REFERENCES devices(id) ON DELETE CASCADE,
    name VARCHAR(255) NOT NULL,
    description TEXT,
    type config_type NOT NULL,
    content TEXT NOT NULL,
    version INTEGER DEFAULT 1,
    is_active BOOLEAN DEFAULT TRUE,
    created_by UUID REFERENCES users(id),
    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    published_at TIMESTAMP WITH TIME ZONE
);

-- 添加索引
CREATE INDEX idx_configs_user_id ON configs(user_id);
CREATE INDEX idx_configs_device_id ON configs(device_id);

-- 添加注释
COMMENT ON TABLE configs IS '设备配置表';
COMMENT ON COLUMN configs.id IS '配置 ID，UUID 格式';
COMMENT ON COLUMN configs.user_id IS '所属用户 ID';
COMMENT ON COLUMN configs.device_id IS '目标设备 ID';
COMMENT ON COLUMN configs.name IS '配置名称';
COMMENT ON COLUMN configs.description IS '配置描述';
COMMENT ON COLUMN configs.type IS '配置类型：network, security, general';
COMMENT ON COLUMN configs.content IS '配置内容，JSON 格式';
COMMENT ON COLUMN configs.version IS '配置版本号';
COMMENT ON COLUMN configs.is_active IS '是否启用';
COMMENT ON COLUMN configs.created_by IS '创建者 ID';
COMMENT ON COLUMN configs.published_at IS '发布时间';

-- +migrate Down
DROP TABLE IF EXISTS configs;
DROP TYPE IF EXISTS config_type;
