USE wheresmybus_v2;

SET NAMES utf8mb4;

START TRANSACTION;

INSERT INTO stops (stop_id, stop_name, stop_type, is_rest_stop) VALUES
    ('S036', 'Nayagaon (Petroleum)', 'STOP', 0),
    ('S037', 'Prem Nagar', 'STOP', 0),
    ('S038', 'Panditwari', 'STOP', 0),
    ('S039', 'Ballupur Chowk', 'STOP', 0),
    ('S040', 'Balliwala', 'STOP', 0),
    ('S041', 'GMS Road', 'STOP', 0),
    ('S042', 'Shimla Bypass Crossing', 'STOP', 0),
    ('S043', 'Transport Nagar', 'STOP', 0),
    ('S044', 'Nehrugram', 'STOP', 0);

INSERT INTO routes (route_id, route_name, description, start_stop_id, rest_stop_id) VALUES
    ('R007', 'Route 7 - Nayagaon', 'Nayagaon to Graphic Era via Prem Nagar, Ballupur, GMS Road and ISBT (morning), same stops in reverse (evening)', 'S036', 'S001'),
    ('R008', 'Route 8 - Nehrugram', 'Nehrugram to Graphic Era via 6 No. Pulia, Rispana Pul and ISBT (morning), same stops in reverse (evening)', 'S044', 'S001');

INSERT INTO route_stops (route_id, stop_order, stop_id, minutes_to_next, km_to_next, eta_is_provisional) VALUES
    ('R007',  1, 'S036',    5, NULL, 1),
    ('R007',  2, 'S037',    5, NULL, 1),
    ('R007',  3, 'S038',    6, NULL, 1),
    ('R007',  4, 'S039',    5, NULL, 1),
    ('R007',  5, 'S040',    4, NULL, 1),
    ('R007',  6, 'S041',    5, NULL, 1),
    ('R007',  7, 'S042',    3, NULL, 1),
    ('R007',  8, 'S043',    2, NULL, 1),
    ('R007',  9, 'S010',    5, NULL, 1),
    ('R007', 10, 'S011',    4, NULL, 1),
    ('R007', 11, 'S001',    4, NULL, 1),
    ('R007', 12, 'S011',    5, NULL, 1),
    ('R007', 13, 'S010',    2, NULL, 1),
    ('R007', 14, 'S043',    3, NULL, 1),
    ('R007', 15, 'S042',    5, NULL, 1),
    ('R007', 16, 'S041',    4, NULL, 1),
    ('R007', 17, 'S040',    5, NULL, 1),
    ('R007', 18, 'S039',    6, NULL, 1),
    ('R007', 19, 'S038',    5, NULL, 1),
    ('R007', 20, 'S037',    5, NULL, 1),
    ('R007', 21, 'S036', NULL, NULL, 0);

INSERT INTO route_stops (route_id, stop_order, stop_id, minutes_to_next, km_to_next, eta_is_provisional) VALUES
    ('R008',  1, 'S044',    4, NULL, 1),
    ('R008',  2, 'S006',    5, NULL, 1),
    ('R008',  3, 'S007',    5, NULL, 1),
    ('R008',  4, 'S008',    4, NULL, 1),
    ('R008',  5, 'S009',    3, NULL, 1),
    ('R008',  6, 'S010',    5, NULL, 1),
    ('R008',  7, 'S011',    4, NULL, 1),
    ('R008',  8, 'S001',    4, NULL, 1),
    ('R008',  9, 'S011',    5, NULL, 1),
    ('R008', 10, 'S010',    3, NULL, 1),
    ('R008', 11, 'S009',    4, NULL, 1),
    ('R008', 12, 'S008',    5, NULL, 1),
    ('R008', 13, 'S007',    5, NULL, 1),
    ('R008', 14, 'S006',    4, NULL, 1),
    ('R008', 15, 'S044', NULL, NULL, 0);

INSERT INTO buses (bus_id, route_id) VALUES
    ('701', 'R007'),
    ('702', 'R007'),
    ('801', 'R008'),
    ('802', 'R008');

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
