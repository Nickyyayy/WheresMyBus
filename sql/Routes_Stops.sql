CREATE DATABASE IF NOT EXISTS where_my_bus
  CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
USE where_my_bus;

DROP TABLE IF EXISTS etas;
DROP TABLE IF EXISTS route_stops;
DROP TABLE IF EXISTS routes;
DROP TABLE IF EXISTS stops;

CREATE TABLE stops (
    stop_id VARCHAR(10) PRIMARY KEY,stop_name VARCHAR(100) NOT NULL UNIQUE,
    stop_type ENUM('COLLEGE','STOP','REST') NOT NULL DEFAULT 'STOP',
    is_rest_stop BOOLEAN NOT NULL DEFAULT FALSE
);

CREATE TABLE routes (
    route_id VARCHAR(10) PRIMARY KEY,
    route_name VARCHAR(100) NOT NULL,description VARCHAR(255),
    start_stop_id VARCHAR(10) NOT NULL,rest_stop_id VARCHAR(10) NOT NULL,
    FOREIGN KEY (start_stop_id) REFERENCES stops(stop_id),
    FOREIGN KEY (rest_stop_id) REFERENCES stops(stop_id)
);

CREATE TABLE route_stops (
    route_id VARCHAR(10) NOT NULL,
    stop_order INT NOT NULL,
    stop_id VARCHAR(10) NOT NULL,
    PRIMARY KEY (route_id, stop_order),
    FOREIGN KEY (route_id) REFERENCES routes(route_id) ON DELETE CASCADE,
    FOREIGN KEY (stop_id) REFERENCES stops(stop_id),
    UNIQUE KEY uq_route_stop_position (route_id, stop_id, stop_order)
);

CREATE TABLE etas (
    eta_id INT AUTO_INCREMENT PRIMARY KEY,
    route_id VARCHAR(10) NOT NULL,
    from_stop_id VARCHAR(10) NOT NULL,
    to_stop_id VARCHAR(10) NOT NULL,
    estimated_minutes INT NOT NULL,
    distance_km DECIMAL(5,2) NULL,
    is_provisional BOOLEAN NOT NULL DEFAULT TRUE,
    FOREIGN KEY (route_id) REFERENCES routes(route_id) ON DELETE CASCADE,
    FOREIGN KEY (from_stop_id) REFERENCES stops(stop_id),
    FOREIGN KEY (to_stop_id) REFERENCES stops(stop_id),
    CHECK (estimated_minutes > 0),
    CHECK (from_stop_id <> to_stop_id)
);

INSERT INTO stops (stop_id, stop_name, stop_type, is_rest_stop) VALUES
('S001', 'Graphic Era', 'COLLEGE', 0),
('S002', 'Clement Town', 'STOP', 0),
('S003', 'Subhasnagar', 'STOP', 0),
('S004', 'ISBT', 'STOP', 0),
('S005', 'Kargi Chowk', 'STOP', 0),
('S006', 'Bangali Kothi Chowk', 'STOP', 0),
('S007', 'Rispana Bridge', 'STOP', 0),
('S008', 'Nehru Colony', 'STOP', 0),
('S009', 'Fountain Chowk', 'STOP', 0),
('S010', 'Ambiwala', 'STOP', 0),
('S011', '6 No Puliya', 'STOP', 0),
('S012', 'Dobhal Chowk', 'STOP', 0),
('S013', 'Raipur Chowk', 'STOP', 0),
('S014', 'Ranjhawala', 'STOP', 0),
('S015', 'Gujronwali Chowk', 'STOP', 0),
('S016', 'Donali', 'REST', 1),
('S017', 'Jogiwala', 'STOP', 0),
('S018', 'Ring Road', 'STOP', 0),
('S019', 'Mokhampur', 'STOP', 0),
('S020', 'Mall of Dehradun', 'STOP', 0),
('S021', 'Maiynawala Chowk', 'STOP', 0),
('S022', 'Balawala Road', 'STOP', 0),
('S023', 'Gullarghati Road', 'STOP', 0),
('S024', 'Mamchand Chowk', 'STOP', 0),
('S025', 'Raipur-Balawala Road', 'STOP', 0);

INSERT INTO routes (route_id, route_name, description, start_stop_id, rest_stop_id) VALUES
('R001', 'Route 1 - Nehru Colony / Ambiwala', 'Nehru Colony branch via Fountain Chowk and Ambiwala', 'S001', 'S016'),
('R002', 'Route 2 - Jogiwala / Ring Road', 'Jogiwala and Ring Road branch', 'S001', 'S016'),
('R003', 'Route 3 - Mokhampur / Balawala', 'Jogiwala, Mokhampur and Balawala branch', 'S001', 'S016');

INSERT INTO route_stops (route_id, stop_order, stop_id) VALUES
-- Route 1: Graphic Era -> ... -> Donali -> Graphic Era
('R001', 1, 'S001'),
('R001', 2, 'S002'),
('R001', 3, 'S003'),
('R001', 4, 'S004'),
('R001', 5, 'S005'),
('R001', 6, 'S006'),
('R001', 7, 'S007'),
('R001', 8, 'S008'),
('R001', 9, 'S009'),
('R001', 10, 'S010'),
('R001', 11, 'S011'),
('R001', 12, 'S012'),
('R001', 13, 'S013'),
('R001', 14, 'S014'),
('R001', 15, 'S015'),
('R001', 16, 'S016'),
('R001', 17, 'S001'),

-- Route 2: Graphic Era -> ... -> Donali -> Graphic Era
('R002', 1, 'S001'),
('R002', 2, 'S002'),
('R002', 3, 'S003'),
('R002', 4, 'S004'),
('R002', 5, 'S005'),
('R002', 6, 'S006'),
('R002', 7, 'S007'),
('R002', 8, 'S017'),
('R002', 9, 'S018'),
('R002', 10, 'S011'),
('R002', 11, 'S012'),
('R002', 12, 'S013'),
('R002', 13, 'S014'),
('R002', 14, 'S015'),
('R002', 15, 'S016'),
('R002', 16, 'S001'),

-- Route 3: Graphic Era -> ... -> Donali -> Graphic Era
('R003', 1, 'S001'),
('R003', 2, 'S002'),
('R003', 3, 'S003'),
('R003', 4, 'S004'),
('R003', 5, 'S005'),
('R003', 6, 'S006'),
('R003', 7, 'S007'),
('R003', 8, 'S017'),
('R003', 9, 'S019'),
('R003', 10, 'S020'),
('R003', 11, 'S021'),
('R003', 12, 'S022'),
('R003', 13, 'S023'),
('R003', 14, 'S024'),
('R003', 15, 'S025'),
('R003', 16, 'S016'),
('R003', 17, 'S001');

INSERT INTO etas
(route_id, from_stop_id, to_stop_id, estimated_minutes, distance_km, is_provisional)
VALUES
-- Route 1: provisional 5-10 minute segment ETAs
('R001','S001','S002',6,NULL,TRUE),
('R001','S002','S003',5,NULL,TRUE),
('R001','S003','S004',6,NULL,TRUE),
('R001','S004','S005',7,NULL,TRUE),
('R001','S005','S006',6,NULL,TRUE),
('R001','S006','S007',7,NULL,TRUE),
('R001','S007','S008',6,NULL,TRUE),
('R001','S008','S009',5,NULL,TRUE),
('R001','S009','S010',7,NULL,TRUE),
('R001','S010','S011',6,NULL,TRUE),
('R001','S011','S012',6,NULL,TRUE),
('R001','S012','S013',7,NULL,TRUE),
('R001','S013','S014',6,NULL,TRUE),
('R001','S014','S015',7,NULL,TRUE),
('R001','S015','S016',6,NULL,TRUE),
('R001','S016','S001',10,NULL,TRUE),

-- Route 2: provisional 5-10 minute segment ETAs
('R002','S001','S002',6,NULL,TRUE),
('R002','S002','S003',5,NULL,TRUE),
('R002','S003','S004',6,NULL,TRUE),
('R002','S004','S005',7,NULL,TRUE),
('R002','S005','S006',6,NULL,TRUE),
('R002','S006','S007',7,NULL,TRUE),
('R002','S007','S017',6,NULL,TRUE),
('R002','S017','S018',7,NULL,TRUE),
('R002','S018','S011',6,NULL,TRUE),
('R002','S011','S012',6,NULL,TRUE),
('R002','S012','S013',7,NULL,TRUE),
('R002','S013','S014',6,NULL,TRUE),
('R002','S014','S015',7,NULL,TRUE),
('R002','S015','S016',6,NULL,TRUE),
('R002','S016','S001',10,NULL,TRUE),

-- Route 3: provisional 5-10 minute segment ETAs
('R003','S001','S002',6,NULL,TRUE),
('R003','S002','S003',5,NULL,TRUE),
('R003','S003','S004',6,NULL,TRUE),
('R003','S004','S005',7,NULL,TRUE),
('R003','S005','S006',6,NULL,TRUE),
('R003','S006','S007',7,NULL,TRUE),
('R003','S007','S017',6,NULL,TRUE),
('R003','S017','S019',7,NULL,TRUE),
('R003','S019','S020',7,NULL,TRUE),
('R003','S020','S021',6,NULL,TRUE),
('R003','S021','S022',7,NULL,TRUE),
('R003','S022','S023',6,NULL,TRUE),
('R003','S023','S024',7,NULL,TRUE),
('R003','S024','S025',6,NULL,TRUE),
('R003','S025','S016',7,NULL,TRUE),
('R003','S016','S001',10,NULL,TRUE);

-- View showing each stop and its next stop/ETA.
CREATE OR REPLACE VIEW route_stop_eta AS
SELECT
    r.route_id,
    r.route_name,
    rs.stop_order,
    s.stop_id,
    s.stop_name,
    e.to_stop_id AS next_stop_id,
    ns.stop_name AS next_stop_name,
    e.estimated_minutes AS eta_to_next_stop
FROM routes r
JOIN route_stops rs ON rs.route_id = r.route_id
JOIN stops s ON s.stop_id = rs.stop_id
LEFT JOIN etas e
       ON e.route_id = rs.route_id
      AND e.from_stop_id = rs.stop_id
LEFT JOIN stops ns ON ns.stop_id = e.to_stop_id
ORDER BY r.route_id, rs.stop_order;

