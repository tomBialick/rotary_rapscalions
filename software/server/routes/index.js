const express = require('express');
const { validationResult } = require('express-validator/check');
const { sanitizeQuery } = require('express-validator/filter');
var router = express.Router();

/* GET home page. */
router.get('/', function(req, res, next) {
  res.render('index', { title: 'Rotary Rapscallions' });
});

/* GET an IP for a given phone number */
router.get('/phone', [sanitizeQuery('phone_number').escape()], function(req, res, next) {
  // Deal with any errors
  const errors = validationResult(req);
  if (!errors.isEmpty()) {
    res.status(422).json({ errors: errors.mapped() });
  }
  else if (!req.query.phone_number) {
    res.status(400).json({errors: ["Invalid Phone Number"]});
  }
  else {
    db.query('SELECT * FROM USERS WHERE PHONE_NUMBER = $1', [req.query.phone_number]).then(data => {
      if (!data[0]) {
        res.status(404).json({errors: ["Phone number is not in use"]});
      }
      else {
        res.status(200).json({body: {ip: data[0].phone_number}});
      }
    }).catch(error => {
      console.log('ERROR:', error);
      res.status(500).json({errors: ["Contact Admin(s)"]});
    })
  }
});

module.exports = router;
