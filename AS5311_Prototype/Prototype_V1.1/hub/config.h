"{\
	'general':\
	{\
		'name':'Device',\
		'instance':1\
	},\
	'components':[\
    {\
      'name':'Digital',\
      'params':'default'\
    },\
		{\
			'name':'LoRa',\
			'params':[255,1,23,3,200]\
		},\
    {\
      'name':'GoogleSheets',\
      'params':[\
        'Goog',\
        7001,\
        '/macros/s/AKfycbzySpouxdaHYh6f1e7DI24i4s8XTGDe-X6d-9uaR7HVrpFNWUt7/exec',\
        '11bmZETLyFutZHwZRrpmsNFy0VrWmJ-myskjLZ6cZS-w',\
/*true to autoname tab*/        true,\
/*not used if previous param is true*/  'testTab2'\
      ]\
    },\
		{\
			'name':'SD',\
			'params':[true,1000,10,'datafile',false]\
		},\
    {\
      'name':'BatchSD',\
      'params': [true, 1000, 10]\
     }\
	]\
}"
