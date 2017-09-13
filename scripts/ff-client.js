var FirefoxClient = require("firefox-client");
 
var client = new FirefoxClient();
 
client.connect(6000, function() {
  client.listTabs(function(err, tabs) {
    console.log("first tab:", tabs[0].url);
    tabs[0].navigateTo("http://youtube.com")
  });

  
});

