USE where_my_bus;

CREATE TABLE buses (
bus_id INT AUTO_INCREMENT PRIMARY KEY,
bus_number VARCHAR(10) NOT NULL UNIQUE,
route_id VARCHAR(10) NOT NULL,

FOREIGN KEY (route_id)
REFERENCES routes(route_id)
);
INSERT INTO buses (bus_number, route_id) VALUES
-- R001
('68', 'R001'),
('78', 'R001'),
('66', 'R001'),
('88', 'R001'),
('118', 'R001'),
('55', 'R001'),
-- R002
('21', 'R002'),
('108', 'R002'),
('77', 'R002'),
('32', 'R002'),
-- R003
('106', 'R003'),
('70', 'R003'),
('42', 'R003'),
('45', 'R003');