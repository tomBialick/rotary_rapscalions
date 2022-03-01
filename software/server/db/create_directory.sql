
-- Run this to set up the postgresql database table
-- Should only need to be ran once

--Drop table(s) before making

DROP TABLE DIRECTORY;

CREATE TABLE DIRECTORY(
      ID            INT          NOT NULL,
      --USERNAME      VARCHAR(255) NOT NULL,
      --PASSWORD      VARCHAR(255) NOT NULL,
      IP_ADDRESS    VARCHAR(255) NOT NULL,
      PHONE_NUMBER  VARCHAR(255) NOT NULL,
      PRIMARY KEY (ID)
);
