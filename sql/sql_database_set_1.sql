SET NAMES utf8mb4;

DROP DATABASE IF EXISTS wheresmybus_v2;
CREATE DATABASE wheresmybus_v2 CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
USE wheresmybus_v2;

CREATE TABLE stops (
    stop_id       VARCHAR(10)  NOT NULL,
    stop_name     VARCHAR(100) NOT NULL,
    stop_type     ENUM('COLLEGE','STOP','REST') NOT NULL DEFAULT 'STOP',
    is_rest_stop  TINYINT(1)   NOT NULL DEFAULT 0,
    PRIMARY KEY (stop_id),
    UNIQUE KEY uq_stop_name (stop_name),
    CONSTRAINT chk_rest_flag CHECK (is_rest_stop IN (0,1))
) ENGINE=InnoDB;

CREATE TABLE routes (
    route_id       VARCHAR(10)  NOT NULL,
    route_name     VARCHAR(100) NOT NULL,
    description    VARCHAR(255) NULL,
    start_stop_id  VARCHAR(10)  NOT NULL,
    rest_stop_id   VARCHAR(10)  NOT NULL,
    PRIMARY KEY (route_id),
    UNIQUE KEY uq_route_name (route_name),
    CONSTRAINT fk_route_start FOREIGN KEY (start_stop_id) REFERENCES stops(stop_id),
    CONSTRAINT fk_route_rest  FOREIGN KEY (rest_stop_id)  REFERENCES stops(stop_id)
) ENGINE=InnoDB;

CREATE TABLE route_stops (
    route_id            VARCHAR(10)  NOT NULL,
    stop_order          INT          NOT NULL,
    stop_id             VARCHAR(10)  NOT NULL,
    minutes_to_next     INT          NULL,
    km_to_next          DECIMAL(5,2) NULL,
    eta_is_provisional  TINYINT(1)   NOT NULL DEFAULT 1,
    PRIMARY KEY (route_id, stop_order),
    KEY idx_route_stops_stop (stop_id),
    CONSTRAINT fk_rs_route FOREIGN KEY (route_id) REFERENCES routes(route_id) ON DELETE CASCADE,
    CONSTRAINT fk_rs_stop  FOREIGN KEY (stop_id)  REFERENCES stops(stop_id),
    CONSTRAINT chk_rs_order   CHECK (stop_order >= 1),
    CONSTRAINT chk_rs_minutes CHECK (minutes_to_next IS NULL OR minutes_to_next > 0),
    CONSTRAINT chk_rs_km      CHECK (km_to_next IS NULL OR km_to_next > 0),
    CONSTRAINT chk_rs_prov    CHECK (eta_is_provisional IN (0,1))
) ENGINE=InnoDB;

CREATE TABLE buses (
    bus_id    VARCHAR(10) NOT NULL,
    route_id  VARCHAR(10) NOT NULL,
    PRIMARY KEY (bus_id),
    KEY idx_buses_route (route_id),
    CONSTRAINT fk_bus_route FOREIGN KEY (route_id) REFERENCES routes(route_id)
) ENGINE=InnoDB;

START TRANSACTION;

INSERT INTO stops (stop_id, stop_name, stop_type, is_rest_stop) VALUES
    ('S001', 'Graphic Era Deemed to be University', 'COLLEGE', 1),
    ('S002', 'Donali', 'STOP', 0),
    ('S003', 'Gujrounwala', 'STOP', 0),
    ('S004', 'Raipur', 'STOP', 0),
    ('S005', 'Dobhal Chowk', 'STOP', 0),
    ('S006', '6 No. Pulia', 'STOP', 0),
    ('S007', 'Fuwara Chowk', 'STOP', 0),
    ('S008', 'Rispana Pul', 'STOP', 0),
    ('S009', 'Kargi Chowk', 'STOP', 0),
    ('S010', 'ISBT', 'STOP', 0),
    ('S011', 'Clement Town', 'STOP', 0),
    ('S012', 'Doiwala', 'STOP', 0),
    ('S013', 'Harrawala', 'STOP', 0),
    ('S014', 'Miyanwala', 'STOP', 0),
    ('S015', 'Mohakampur', 'STOP', 0),
    ('S016', 'Jogiwala', 'STOP', 0);

INSERT INTO routes (route_id, route_name, description, start_stop_id, rest_stop_id) VALUES
    ('R001', 'Route 1 - Donali',  'Donali to Graphic Era via Raipur and ISBT (morning), same stops in reverse (evening)', 'S002', 'S001'),
    ('R002', 'Route 2 - Doiwala', 'Doiwala to Graphic Era via Jogiwala and ISBT (morning), same stops in reverse (evening)', 'S012', 'S001');

INSERT INTO route_stops (route_id, stop_order, stop_id, minutes_to_next, km_to_next, eta_is_provisional) VALUES
    ('R001',  1, 'S002',    1, NULL, 0),
    ('R001',  2, 'S003',    6, NULL, 1),
    ('R001',  3, 'S004',    5, NULL, 1),
    ('R001',  4, 'S005',    5, NULL, 1),
    ('R001',  5, 'S006',    5, NULL, 1),
    ('R001',  6, 'S007',    5, NULL, 1),
    ('R001',  7, 'S008',    4, NULL, 1),
    ('R001',  8, 'S009',    3, NULL, 1),
    ('R001',  9, 'S010',    5, NULL, 1),
    ('R001', 10, 'S011',    4, NULL, 1),
    ('R001', 11, 'S001',    4, NULL, 1),
    ('R001', 12, 'S011',    5, NULL, 1),
    ('R001', 13, 'S010',    3, NULL, 1),
    ('R001', 14, 'S009',    4, NULL, 1),
    ('R001', 15, 'S008',    5, NULL, 1),
    ('R001', 16, 'S007',    5, NULL, 1),
    ('R001', 17, 'S006',    5, NULL, 1),
    ('R001', 18, 'S005',    5, NULL, 1),
    ('R001', 19, 'S004',    6, NULL, 1),
    ('R001', 20, 'S003',    1, NULL, 0),
    ('R001', 21, 'S002', NULL, NULL, 0);

INSERT INTO route_stops (route_id, stop_order, stop_id, minutes_to_next, km_to_next, eta_is_provisional) VALUES
    ('R002',  1, 'S012',    5, NULL, 1),
    ('R002',  2, 'S013',    4, NULL, 1),
    ('R002',  3, 'S014',    3, NULL, 1),
    ('R002',  4, 'S015',    2, NULL, 1),
    ('R002',  5, 'S016',    3, NULL, 1),
    ('R002',  6, 'S008',    4, NULL, 1),
    ('R002',  7, 'S009',    3, NULL, 1),
    ('R002',  8, 'S010',    5, NULL, 1),
    ('R002',  9, 'S011',    4, NULL, 1),
    ('R002', 10, 'S001',    4, NULL, 1),
    ('R002', 11, 'S011',    5, NULL, 1),
    ('R002', 12, 'S010',    3, NULL, 1),
    ('R002', 13, 'S009',    4, NULL, 1),
    ('R002', 14, 'S008',    3, NULL, 1),
    ('R002', 15, 'S016',    2, NULL, 1),
    ('R002', 16, 'S015',    3, NULL, 1),
    ('R002', 17, 'S014',    4, NULL, 1),
    ('R002', 18, 'S013',    5, NULL, 1),
    ('R002', 19, 'S012', NULL, NULL, 0);

INSERT INTO buses (bus_id, route_id) VALUES
    ('101', 'R001'),
    ('102', 'R001'),
    ('201', 'R002'),
    ('202', 'R002');

COMMIT;

SELECT route_id, COUNT(*) AS positions, SUM(minutes_to_next) AS round_trip_min
FROM route_stops
GROUP BY route_id
ORDER BY route_id;

SELECT route_id AS gap_in_stop_order
FROM route_stops
GROUP BY route_id
HAVING MIN(stop_order) <> 1 OR MAX(stop_order) <> COUNT(*);

SELECT rs.route_id AS bad_terminus_route, rs.stop_order
FROM route_stops rs
JOIN (SELECT route_id, MAX(stop_order) AS last_order
      FROM route_stops GROUP BY route_id) m ON m.route_id = rs.route_id
WHERE (rs.stop_order = m.last_order) <> (rs.minutes_to_next IS NULL);

SELECT r.route_id AS wrong_first_stop
FROM routes r
LEFT JOIN route_stops rs ON rs.route_id = r.route_id AND rs.stop_order = 1
WHERE rs.stop_id IS NULL OR rs.stop_id <> r.start_stop_id;

SELECT r.route_id AS rest_stop_problem, r.rest_stop_id
FROM routes r
JOIN stops s ON s.stop_id = r.rest_stop_id
WHERE s.is_rest_stop <> 1
   OR NOT EXISTS (SELECT 1 FROM route_stops rs
                  WHERE rs.route_id = r.route_id AND rs.stop_id = r.rest_stop_id);

SELECT 'route without buses' AS orphan, r.route_id AS id
FROM routes r
WHERE NOT EXISTS (SELECT 1 FROM buses b WHERE b.route_id = r.route_id)
UNION ALL
SELECT 'stop on no route', s.stop_id
FROM stops s
WHERE NOT EXISTS (SELECT 1 FROM route_stops rs WHERE rs.stop_id = s.stop_id);
