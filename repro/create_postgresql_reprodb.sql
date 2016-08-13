
-- Login as the postgres admin user (normally called postgres)
--
--    createdb repro
--    createuser -D -R -S -P repro
--
-- and set up network access to the repro database through
-- /etc/postgresql/9.1/main/pg_hba.conf or equivalent


-- Uncomment the following to have all tables re-created
-- DROP TABLE IF EXISTS users;
-- DROP TABLE IF EXISTS routesavp;
-- DROP TABLE IF EXISTS aclsavp;
-- DROP TABLE IF EXISTS configsavp;
-- DROP TABLE IF EXISTS staticregsavp;
-- DROP TABLE IF EXISTS filtersavp;
-- DROP TABLE IF EXISTS siloavp;


--
-- Table structure for table users
--
CREATE TABLE IF NOT EXISTS users (
  id SERIAL PRIMARY KEY,
  username VARCHAR(64) NOT NULL,
  domain VARCHAR(253),
  realm VARCHAR(253),
  passwordHash VARCHAR(32),
  passwordHashAlt VARCHAR(32),
  name VARCHAR(256),
  email VARCHAR(256),
  forwardAddress VARCHAR(256)
);

CREATE UNIQUE INDEX idx_user_domain ON users (username, domain);

--
-- Table structure for table routesavp
--

CREATE TABLE IF NOT EXISTS routesavp (
  method VARCHAR(255) NOT NULL,
  event VARCHAR(255),
  matchingpattern VARCHAR(255),
  rewriteexpression VARCHAR(255),
  order VARCHAR(255),
  value VARCHAR(4096),
  PRIMARY KEY (method, event, matchingpattern, order)
);

--
-- Table structure for table aclsavp
--

CREATE TABLE IF NOT EXISTS aclsavp (
  tlspeername VARCHAR(255) NOT NULL,
  address VARCHAR(255),
  mask VARCHAR(255),
  port VARCHAR(255),
  family VARCHAR(255),
  transport VARCHAR(255),
  value VARCHAR(4096),
  PRIMARY KEY (tlspeername, address, mask, port, family, transport)
);

--
-- Table structure for table configsavp
--

CREATE TABLE IF NOT EXISTS configsavp (
  domain VARCHAR(255) NOT NULL,
  tlsport VARCHAR(255),
  value VARCHAR(4096),
  PRIMARY KEY (domain, tlsport)
);

--
-- Table structure for table staticregsavp
--

CREATE TABLE IF NOT EXISTS staticregsavp (
  aor VARCHAR(255) NOT NULL,
  contact VARCHAR(255),
  mpath VARCHAR(255), 
  value VARCHAR(4096),
  PRIMARY KEY (aor, contact, mpath)
);

--
-- Table structure for table filtersavp
--

CREATE TABLE IF NOT EXISTS filtersavp (
  cond1header VARCHAR(255) NOT NULL,
  cond1regex VARCHAR(255),
  cond2header VARCHAR(255),
  cond2regex VARCHAR(255),
  method VARCHAR(255),
  event VARCHAR(255),
  value VARCHAR(4096),
  PRIMARY KEY (cond1header, cond1regex, cond2header, cond2regex, method, event)
);

--
-- Table structure for table siloavp
--

CREATE TABLE IF NOT EXISTS siloavp (
  desturi VARCHAR(255) NOT NULL,
  sourceuri VARCHAR(255) NOT NULL,
  sendtime VARCHAR(24),
  tid VARCHAR(255),
  mimetype VARCHAR(255),
  msgbody VARCHAR(255),
  value VARCHAR(20315),
  PRIMARY KEY (desturi, sourceuri, sendtime, tid, mimetype, msgbody)
);

GRANT SELECT, INSERT, UPDATE, DELETE ON users, routesavp,
  aclsavp, configsavp, staticregsavp, filtersavp, siloavp TO repro;
GRANT USAGE, SELECT, UPDATE ON users_id_seq TO repro;

