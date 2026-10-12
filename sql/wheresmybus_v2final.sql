-- =====================================================================
-- WheresMyBus  -  Database v2 (full replacement of the old schema)
-- Target: MySQL 8.0.16+ (CHECK constraints are ignored on older versions)
--
-- Design change vs v1: the separate `etas` table is gone. A travel time
-- belongs to a POSITION on a route (leg i -> i+1), not to a pair of stops,
-- because stop pairs repeat on round trips. Each route_stops row stores the
-- time/distance to the NEXT row, which maps 1:1 onto RouteStopNode.
--
-- Data notes:
--  * Routes are round trips: start stop -> Graphic Era (rest stop, buses
--    wait during classes) -> same stops in reverse -> start stop.
--  * Minutes are ESTIMATES split from the stated totals (Route 1 ~43 min,
--    Route 2 ~33 min one way). eta_is_provisional = 1 marks estimates;
--    only Donali <-> Gujrounwala (1 min) is a user-supplied figure.
--  * km_to_next is NULL everywhere: no distance data was available.
--  * Bus numbers 101/102/201/202 are PLACEHOLDERS - replace with real ones.
--
-- This creates a NEW database, so the old one is left untouched.
-- Point the database name in config.txt at wheresmybus_v2 to use it.
--
-- Load it WITHOUT a database name on the command line, e.g.
--   mysql -u root -p < wheresmybus_v2.sql
-- (passing wheresmybus_v2 as an argument fails on the first run, because
-- the database does not exist until this script creates it).
--
-- Re-running this script DROPS and rebuilds wheresmybus_v2 from scratch.
-- The CHECKS section at the end runs automatically and reports problems.
-- =====================================================================

SET NAMES utf8mb4;

DROP DATABASE IF EXISTS wheresmybus_v2;
CREATE DATABASE wheresmybus_v2 CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
USE wheresmybus_v2;

-- ---------------------------------------------------------------------
-- Schema
-- ---------------------------------------------------------------------

CREATE TABLE stops (
    stop_id       VARCHAR(10)  NOT NULL,
    stop_name     VARCHAR(100) NOT NULL,
    stop_type     ENUM('COLLEGE','STOP','REST') NOT NULL DEFAULT 'STOP',
    is_rest_stop  TINYINT(1)   NOT NULL DEFAULT 0,  -- kept separate from stop_type:
                                                    -- the college is also the rest stop
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

-- One row per position on a route. The last row of a route has NULL
-- minutes/km (terminus). Contiguous ordering (1,2,3,...) and "route starts
-- at start_stop_id / visits rest_stop_id" are checked in RouteValidation.cpp.
CREATE TABLE route_stops (
    route_id            VARCHAR(10)  NOT NULL,
    stop_order          INT          NOT NULL,      -- 1-based position
    stop_id             VARCHAR(10)  NOT NULL,
    minutes_to_next     INT          NULL,          -- NULL on the terminus row
    km_to_next          DECIMAL(5,2) NULL,          -- NULL = unknown
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

-- All data below is inserted as one unit: if any row fails,
-- none of the data is kept.
START TRANSACTION;

-- ---------------------------------------------------------------------
-- Data: stops (16 distinct)
-- ---------------------------------------------------------------------
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

-- ---------------------------------------------------------------------
-- Data: routes
-- ---------------------------------------------------------------------
INSERT INTO routes (route_id, route_name, description, start_stop_id, rest_stop_id) VALUES
    ('R001', 'Route 1 - Donali',  'Donali to Graphic Era via Raipur and ISBT (morning), same stops in reverse (evening)', 'S002', 'S001'),
    ('R002', 'Route 2 - Doiwala', 'Doiwala to Graphic Era via Jogiwala and ISBT (morning), same stops in reverse (evening)', 'S012', 'S001');

-- ---------------------------------------------------------------------
-- Data: Route 1: Donali <-> Graphic Era  (21 positions, 86 min round trip)
-- ---------------------------------------------------------------------
INSERT INTO route_stops (route_id, stop_order, stop_id, minutes_to_next, km_to_next, eta_is_provisional) VALUES
    ('R001',  1, 'S002',    1, NULL, 0),  -- Donali -> Gujrounwala
    ('R001',  2, 'S003',    6, NULL, 1),  -- Gujrounwala -> Raipur
    ('R001',  3, 'S004',    5, NULL, 1),  -- Raipur -> Dobhal Chowk
    ('R001',  4, 'S005',    5, NULL, 1),  -- Dobhal Chowk -> 6 No. Pulia
    ('R001',  5, 'S006',    5, NULL, 1),  -- 6 No. Pulia -> Fuwara Chowk
    ('R001',  6, 'S007',    5, NULL, 1),  -- Fuwara Chowk -> Rispana Pul
    ('R001',  7, 'S008',    4, NULL, 1),  -- Rispana Pul -> Kargi Chowk
    ('R001',  8, 'S009',    3, NULL, 1),  -- Kargi Chowk -> ISBT
    ('R001',  9, 'S010',    5, NULL, 1),  -- ISBT -> Clement Town
    ('R001', 10, 'S011',    4, NULL, 1),  -- Clement Town -> Graphic Era Deemed to be University
    ('R001', 11, 'S001',    4, NULL, 1),  -- Graphic Era Deemed to be University -> Clement Town
    ('R001', 12, 'S011',    5, NULL, 1),  -- Clement Town -> ISBT
    ('R001', 13, 'S010',    3, NULL, 1),  -- ISBT -> Kargi Chowk
    ('R001', 14, 'S009',    4, NULL, 1),  -- Kargi Chowk -> Rispana Pul
    ('R001', 15, 'S008',    5, NULL, 1),  -- Rispana Pul -> Fuwara Chowk
    ('R001', 16, 'S007',    5, NULL, 1),  -- Fuwara Chowk -> 6 No. Pulia
    ('R001', 17, 'S006',    5, NULL, 1),  -- 6 No. Pulia -> Dobhal Chowk
    ('R001', 18, 'S005',    5, NULL, 1),  -- Dobhal Chowk -> Raipur
    ('R001', 19, 'S004',    6, NULL, 1),  -- Raipur -> Gujrounwala
    ('R001', 20, 'S003',    1, NULL, 0),  -- Gujrounwala -> Donali
    ('R001', 21, 'S002', NULL, NULL, 0);  -- Donali (terminus)

-- ---------------------------------------------------------------------
-- Data: Route 2: Doiwala <-> Graphic Era  (19 positions, 66 min round trip)
-- ---------------------------------------------------------------------
INSERT INTO route_stops (route_id, stop_order, stop_id, minutes_to_next, km_to_next, eta_is_provisional) VALUES
    ('R002',  1, 'S012',    5, NULL, 1),  -- Doiwala -> Harrawala
    ('R002',  2, 'S013',    4, NULL, 1),  -- Harrawala -> Miyanwala
    ('R002',  3, 'S014',    3, NULL, 1),  -- Miyanwala -> Mohakampur
    ('R002',  4, 'S015',    2, NULL, 1),  -- Mohakampur -> Jogiwala
    ('R002',  5, 'S016',    3, NULL, 1),  -- Jogiwala -> Rispana Pul
    ('R002',  6, 'S008',    4, NULL, 1),  -- Rispana Pul -> Kargi Chowk
    ('R002',  7, 'S009',    3, NULL, 1),  -- Kargi Chowk -> ISBT
    ('R002',  8, 'S010',    5, NULL, 1),  -- ISBT -> Clement Town
    ('R002',  9, 'S011',    4, NULL, 1),  -- Clement Town -> Graphic Era Deemed to be University
    ('R002', 10, 'S001',    4, NULL, 1),  -- Graphic Era Deemed to be University -> Clement Town
    ('R002', 11, 'S011',    5, NULL, 1),  -- Clement Town -> ISBT
    ('R002', 12, 'S010',    3, NULL, 1),  -- ISBT -> Kargi Chowk
    ('R002', 13, 'S009',    4, NULL, 1),  -- Kargi Chowk -> Rispana Pul
    ('R002', 14, 'S008',    3, NULL, 1),  -- Rispana Pul -> Jogiwala
    ('R002', 15, 'S016',    2, NULL, 1),  -- Jogiwala -> Mohakampur
    ('R002', 16, 'S015',    3, NULL, 1),  -- Mohakampur -> Miyanwala
    ('R002', 17, 'S014',    4, NULL, 1),  -- Miyanwala -> Harrawala
    ('R002', 18, 'S013',    5, NULL, 1),  -- Harrawala -> Doiwala
    ('R002', 19, 'S012', NULL, NULL, 0);  -- Doiwala (terminus)

-- ---------------------------------------------------------------------
-- Data: buses  (PLACEHOLDER numbers - replace with the real bus numbers)
-- ---------------------------------------------------------------------
INSERT INTO buses (bus_id, route_id) VALUES
    ('101', 'R001'),
    ('102', 'R001'),
    ('201', 'R002'),
    ('202', 'R002');

COMMIT;

-- ---------------------------------------------------------------------
-- CHECKS (run automatically on every load)
-- Check 1 prints a summary. Checks 2-6 print NOTHING when the data is
-- correct; any row they print is a problem to fix.
-- ---------------------------------------------------------------------

-- Check 1: summary (expect R001 = 21 positions / 86 min, R002 = 19 / 66)
SELECT route_id, COUNT(*) AS positions, SUM(minutes_to_next) AS round_trip_min
FROM route_stops
GROUP BY route_id
ORDER BY route_id;

-- Check 2: positions must be exactly 1, 2, 3, ... with no gaps
SELECT route_id AS gap_in_stop_order
FROM route_stops
GROUP BY route_id
HAVING MIN(stop_order) <> 1 OR MAX(stop_order) <> COUNT(*);

-- Check 3: only the LAST position may have NULL minutes, and it must
SELECT rs.route_id AS bad_terminus_route, rs.stop_order
FROM route_stops rs
JOIN (SELECT route_id, MAX(stop_order) AS last_order
      FROM route_stops GROUP BY route_id) m ON m.route_id = rs.route_id
WHERE (rs.stop_order = m.last_order) <> (rs.minutes_to_next IS NULL);

-- Check 4: position 1 must be the route's start_stop_id
SELECT r.route_id AS wrong_first_stop
FROM routes r
LEFT JOIN route_stops rs ON rs.route_id = r.route_id AND rs.stop_order = 1
WHERE rs.stop_id IS NULL OR rs.stop_id <> r.start_stop_id;

-- Check 5: every route must pass through its rest stop, and that stop
-- must be flagged is_rest_stop = 1
SELECT r.route_id AS rest_stop_problem, r.rest_stop_id
FROM routes r
JOIN stops s ON s.stop_id = r.rest_stop_id
WHERE s.is_rest_stop <> 1
   OR NOT EXISTS (SELECT 1 FROM route_stops rs
                  WHERE rs.route_id = r.route_id AND rs.stop_id = r.rest_stop_id);

-- Check 6: every route needs at least one bus, every stop at least one route
SELECT 'route without buses' AS orphan, r.route_id AS id
FROM routes r
WHERE NOT EXISTS (SELECT 1 FROM buses b WHERE b.route_id = r.route_id)
UNION ALL
SELECT 'stop on no route', s.stop_id
FROM stops s
WHERE NOT EXISTS (SELECT 1 FROM route_stops rs WHERE rs.stop_id = s.stop_id);
