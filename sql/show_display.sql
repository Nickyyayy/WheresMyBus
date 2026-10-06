USE where_my_bus;

SHOW TABLES;

SELECT * FROM stops;

SELECT * FROM routes;

SELECT * FROM route_stops;

SELECT * FROM etas;

-- everything all columns
SELECT * FROM route_stop_eta;

DESCRIBE stops;
DESCRIBE routes;
DESCRIBE route_stops;
DESCRIBE etas;

SELECT * FROM buses;

SELECT 
buses.bus_number,
routes.route_name
FROM buses
JOIN routes
ON buses.route_id = routes.route_id
ORDER BY buses.route_id, buses.bus_number; 

