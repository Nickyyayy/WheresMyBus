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
('45', 'R003'),

-- R004
('401', 'R004'),
('402', 'R004'),
-- R005
('501', 'R005'),
('502', 'R005'),
-- R006
('601', 'R006'),
('602', 'R006'),
-- R007
('701', 'R007'),
('702', 'R007'),
-- R008
('801', 'R008'),
('802', 'R008'),
-- R009
('301', 'R009'),
('302', 'R009');