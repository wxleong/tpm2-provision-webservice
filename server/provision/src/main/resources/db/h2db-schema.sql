
CREATE TABLE IF NOT EXISTS tpm2_session (
    id BIGINT AUTO_INCREMENT PRIMARY KEY,
    rc INT,
    rc_message TEXT,
    command TEXT,
    response TEXT,
    status INT
);