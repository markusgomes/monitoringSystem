
-- Criação da Tabela de Usuários
CREATE TABLE usuarios (
    id BIGSERIAL PRIMARY KEY,
    nome TEXT NOT NULL,
    senha TEXT NOT NULL,
    email TEXT UNIQUE NOT NULL,
    situacao TEXT NOT NULL,
    data_hora TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
);

-- Criação da Tabela de Sessões
CREATE TABLE sessoes (
    id BIGSERIAL PRIMARY KEY,
    usuario_id BIGINT REFERENCES usuarios(id),
    duracao INTEGER NOT NULL,
    sensor_dht BOOLEAN NOT NULL DEFAULT false,
    sensor_max BOOLEAN NOT NULL DEFAULT false,
    data_hora TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
);

-- Criação da Tabela do Sensor DHT22
CREATE TABLE dht22 (
    id BIGSERIAL PRIMARY KEY,
    sessao_id BIGINT REFERENCES sessoes(id),
    temperatura REAL NOT NULL,
    umidade REAL NOT NULL,
    data_hora TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
);

-- Criação da Tabela do Sensor MAX9814
CREATE TABLE max9814 (
    id BIGSERIAL PRIMARY KEY,
    sessao_id BIGINT REFERENCES sessoes(id),
    maximo REAL NOT NULL,
    minimo REAL NOT NULL,
    data_hora TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
);

-- Criação da Tabela do Sensor Controle DHT22
CREATE TABLE dht22Controle (
    id BIGSERIAL PRIMARY KEY,
    sessao_id BIGINT REFERENCES sessoes(id),
    temperatura REAL NOT NULL,
    umidade REAL NOT NULL,
    data_hora TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
);

-- Criação da Tabela do Sensor Controle MAX9814
CREATE TABLE max9814Controle (
    id BIGSERIAL PRIMARY KEY,
    sessao_id BIGINT REFERENCES sessoes(id),
    maximo REAL NOT NULL,
    minimo REAL NOT NULL,
    data_hora TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
);
