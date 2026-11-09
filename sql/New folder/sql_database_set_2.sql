USE wheresmybus_v2;

SET NAMES utf8mb4;

START TRANSACTION;

INSERT INTO stops (stop_id, stop_name, stop_type, is_rest_stop) VALUES
    ('S017', 'Sahastradhara', 'STOP', 0),
    ('S018', 'Kulhan', 'STOP', 0),
    ('S019', 'IT Park', 'STOP', 0),
    ('S020', 'Sahastradhara Crossing', 'STOP', 0),
    ('S021', 'Ladpur', 'STOP', 0),
    ('S022', 'Rajpur', 'STOP', 0),
    ('S023', 'Sai Mandir', 'STOP', 0),
    ('S024', 'Mussoorie Diversion', 'STOP', 0),
    ('S025', 'Jakhan', 'STOP', 0),
    ('S026', 'Dilaram Chowk', 'STOP', 0),
    ('S027', 'Clock Tower', 'STOP', 0),
    ('S028', 'Prince Chowk', 'STOP', 0),
    ('S029', 'Saharanpur Chowk', 'STOP', 0),
    ('S030', 'Lal Pul', 'STOP', 0),
    ('S031', 'Rishikesh (Natraj Chowk)', 'STOP', 0),
    ('S032', 'Bhogpur', 'STOP', 0),
    ('S033', 'Jolly Grant', 'STOP', 0),
    ('S034', 'Bhaniyawala', 'STOP', 0),
    ('S035', 'Graphic Era Hill University Hostel', 'STOP', 0);

INSERT INTO routes (route_id, route_name, description, start_stop_id, rest_stop_id) VALUES
    ('R003', 'Route 3 - Sahastradhara', 'Sahastradhara to Graphic Era via IT Park and Rispana Pul (morning), same stops in reverse (evening)', 'S017', 'S001'),
    ('R004', 'Route 4 - Rajpur Road', 'Rajpur to Graphic Era via Rajpur Road, Clock Tower and ISBT (morning), same stops in reverse (evening)', 'S022', 'S001'),
    ('R005', 'Route 5 - Rishikesh', 'Rishikesh to Graphic Era via Jolly Grant, Doiwala and ISBT (morning), same stops in reverse (evening)', 'S031', 'S001'),
    ('R006', 'Route 6 - GEHU Hostel', 'Graphic Era Hill University Hostel to Graphic Era (morning), and back (evening)', 'S035', 'S001');

INSERT INTO route_stops (route_id, stop_order, stop_id, minutes_to_next, km_to_next, eta_is_provisional) VALUES
    ('R003',  1, 'S017',    7, NULL, 1),
    ('R003',  2, 'S018',    8, NULL, 1),
    ('R003',  3, 'S019',   10, NULL, 1),
    ('R003',  4, 'S020',    7, NULL, 1),
    ('R003',  5, 'S021',    4, NULL, 1),
    ('R003',  6, 'S006',    5, NULL, 1),
    ('R003',  7, 'S007',    5, NULL, 1),
    ('R003',  8, 'S008',    4, NULL, 1),
    ('R003',  9, 'S009',    3, NULL, 1),
    ('R003', 10, 'S010',    5, NULL, 1),
    ('R003', 11, 'S011',    4, NULL, 1),
    ('R003', 12, 'S001',    4, NULL, 1),
    ('R003', 13, 'S011',    5, NULL, 1),
    ('R003', 14, 'S010',    3, NULL, 1),
    ('R003', 15, 'S009',    4, NULL, 1),
    ('R003', 16, 'S008',    5, NULL, 1),
    ('R003', 17, 'S007',    5, NULL, 1),
    ('R003', 18, 'S006',    4, NULL, 1),
    ('R003', 19, 'S021',    7, NULL, 1),
    ('R003', 20, 'S020',   10, NULL, 1),
    ('R003', 21, 'S019',    8, NULL, 1),
    ('R003', 22, 'S018',    7, NULL, 1),
    ('R003', 23, 'S017', NULL, NULL, 0);

INSERT INTO route_stops (route_id, stop_order, stop_id, minutes_to_next, km_to_next, eta_is_provisional) VALUES
    ('R004',  1, 'S022',    3, NULL, 1),
    ('R004',  2, 'S023',    3, NULL, 1),
    ('R004',  3, 'S024',    4, NULL, 1),
    ('R004',  4, 'S025',   10, NULL, 1),
    ('R004',  5, 'S026',    7, NULL, 1),
    ('R004',  6, 'S027',    5, NULL, 1),
    ('R004',  7, 'S028',    5, NULL, 1),
    ('R004',  8, 'S029',    5, NULL, 1),
    ('R004',  9, 'S030',    9, NULL, 1),
    ('R004', 10, 'S010',    5, NULL, 1),
    ('R004', 11, 'S011',    4, NULL, 1),
    ('R004', 12, 'S001',    4, NULL, 1),
    ('R004', 13, 'S011',    5, NULL, 1),
    ('R004', 14, 'S010',    9, NULL, 1),
    ('R004', 15, 'S030',    5, NULL, 1),
    ('R004', 16, 'S029',    5, NULL, 1),
    ('R004', 17, 'S028',    5, NULL, 1),
    ('R004', 18, 'S027',    7, NULL, 1),
    ('R004', 19, 'S026',   10, NULL, 1),
    ('R004', 20, 'S025',    4, NULL, 1),
    ('R004', 21, 'S024',    3, NULL, 1),
    ('R004', 22, 'S023',    3, NULL, 1),
    ('R004', 23, 'S022', NULL, NULL, 0);

INSERT INTO route_stops (route_id, stop_order, stop_id, minutes_to_next, km_to_next, eta_is_provisional) VALUES
    ('R005',  1, 'S031',   20, NULL, 1),
    ('R005',  2, 'S032',   13, NULL, 1),
    ('R005',  3, 'S033',    3, NULL, 1),
    ('R005',  4, 'S034',    5, NULL, 1),
    ('R005',  5, 'S012',    5, NULL, 1),
    ('R005',  6, 'S013',    4, NULL, 1),
    ('R005',  7, 'S014',    3, NULL, 1),
    ('R005',  8, 'S015',    2, NULL, 1),
    ('R005',  9, 'S016',    3, NULL, 1),
    ('R005', 10, 'S008',    4, NULL, 1),
    ('R005', 11, 'S009',    3, NULL, 1),
    ('R005', 12, 'S010',    5, NULL, 1),
    ('R005', 13, 'S011',    4, NULL, 1),
    ('R005', 14, 'S001',    4, NULL, 1),
    ('R005', 15, 'S011',    5, NULL, 1),
    ('R005', 16, 'S010',    3, NULL, 1),
    ('R005', 17, 'S009',    4, NULL, 1),
    ('R005', 18, 'S008',    3, NULL, 1),
    ('R005', 19, 'S016',    2, NULL, 1),
    ('R005', 20, 'S015',    3, NULL, 1),
    ('R005', 21, 'S014',    4, NULL, 1),
    ('R005', 22, 'S013',    5, NULL, 1),
    ('R005', 23, 'S012',    5, NULL, 1),
    ('R005', 24, 'S034',    3, NULL, 1),
    ('R005', 25, 'S033',   13, NULL, 1),
    ('R005', 26, 'S032',   20, NULL, 1),
    ('R005', 27, 'S031', NULL, NULL, 0);

INSERT INTO route_stops (route_id, stop_order, stop_id, minutes_to_next, km_to_next, eta_is_provisional) VALUES
    ('R006',  1, 'S035',    6, NULL, 1),
    ('R006',  2, 'S001',    6, NULL, 1),
    ('R006',  3, 'S035', NULL, NULL, 0);

INSERT INTO buses (bus_id, route_id) VALUES
    ('301', 'R003'),
    ('302', 'R003'),
    ('401', 'R004'),
    ('402', 'R004'),
    ('501', 'R005'),
    ('502', 'R005'),
    ('601', 'R006'),
    ('602', 'R006');

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
