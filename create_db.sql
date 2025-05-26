
-- Criação da Tabela de Usuários
CREATE TABLE usuarios (
    id SERIAL PRIMARY KEY,
    nome TEXT NOT NULL,
    senha TEXT NOT NULL,
    email TEXT UNIQUE NOT NULL,
    situacao TEXT NOT NULL,
    data_hora TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
);

-- Criação da Tabela de Sessões
CREATE TABLE sessoes (
    id SERIAL PRIMARY KEY,
    usuario_id INTEGER REFERENCES usuarios(id),
    duracao INTEGER NOT NULL,
    data_hora TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
);

-- Criação da Tabela do Sensor DHT22
CREATE TABLE dht22 (
    id SERIAL PRIMARY KEY,
    sessao_id INTEGER REFERENCES sessoes(id),
    temperatura REAL NOT NULL,
    umidade REAL NOT NULL,
    data_hora TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
);

-- Criação da Tabela do Sensor MAX9814
CREATE TABLE max9814 (
    id SERIAL PRIMARY KEY,
    sessao_id INTEGER REFERENCES sessoes(id),
    maximo REAL NOT NULL,
    minimo REAL NOT NULL,
    data_hora TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
);

-- Criação da Tabela do Sensor Controle DHT22
CREATE TABLE dht22Controle (
    id SERIAL PRIMARY KEY,
    sessao_id INTEGER REFERENCES sessoes(id),
    temperatura REAL NOT NULL,
    umidade REAL NOT NULL,
    data_hora TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
);

-- Criação da Tabela do Sensor Controle MAX9814
CREATE TABLE max9814Controle (
    id SERIAL PRIMARY KEY,
    sessao_id INTEGER REFERENCES sessoes(id),
    maximo REAL NOT NULL,
    minimo REAL NOT NULL,
    data_hora TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
);
