-- BACKUP AUTOMATICO DOM-US: 2026-08-28 09:37:47

-- TABELLA: sensor_data
TRUNCATE TABLE sensor_data;

INSERT INTO sensor_data (id, created_at, temperatura, umidita, evento) VALUES (1, '2026-08-28 12:00:00', 24.5, 58.2, 'test_python');
INSERT INTO sensor_data (id, created_at, temperatura, umidita, evento) VALUES (3, '2026-08-28 10:24:39', 24.8, 57.5, 'invio_1');
INSERT INTO sensor_data (id, created_at, temperatura, umidita, evento) VALUES (4, '2026-08-28 10:24:39', 25.1, 56.8, 'invio_2');

-- TABELLA: device_status
TRUNCATE TABLE device_status;

INSERT INTO device_status (id, last_ping) VALUES (1, '2026-08-28T10:24:40+00:00');

-- TABELLA: logs
TRUNCATE TABLE logs;

INSERT INTO logs (id, created_at, color, message) VALUES (1, '2026-08-28 12:00:00', 'GREEN', 'Test di sistema da Pi400');
INSERT INTO logs (id, created_at, color, message) VALUES (2, '2026-08-28 10:24:40', 'GREEN', 'Sistema dom-us operativo');

