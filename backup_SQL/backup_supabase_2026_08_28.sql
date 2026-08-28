-- BACKUP AUTOMATICO DOM-US (sensor_data): 2026-08-28 09:48:58

-- TABELLA: sensor_data
TRUNCATE TABLE sensor_data;

INSERT INTO sensor_data (id, created_at, temperatura, umidita, evento) VALUES (1, '2026-08-28 12:00:00', 24.5, 58.2, 'test_python');
INSERT INTO sensor_data (id, created_at, temperatura, umidita, evento) VALUES (3, '2026-08-28 10:24:39', 24.8, 57.5, 'invio_1');
INSERT INTO sensor_data (id, created_at, temperatura, umidita, evento) VALUES (4, '2026-08-28 10:24:39', 25.1, 56.8, 'invio_2');

