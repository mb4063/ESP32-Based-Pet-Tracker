// Complete Project Details at: https://RandomNerdTutorials.com/

// Database Paths
var dataGPSLatPath = 'gps/latitude';
var dataGPSLonPath = 'gps/longitude';
var dataIntPath = 'test/int';
var cameraPath = 'camera/bool';

// Get a database reference 
const databaseGPSLat = database.ref(dataGPSLatPath);
const databaseGPSLong = database.ref(dataGPSLonPath);
const databaseInt = database.ref(dataIntPath);
const databaseCam = database.ref(cameraPath);

// Variables to save database current values
var GPSLatReading;
var GPSLongReading;
var intReading;
var camReading;

// Attach an asynchronous callback to read the data
databaseGPSLat.on('value', (snapshot) => {
    GPSLatReading = snapshot.val();
  console.log(GPSLatReading);
  document.getElementById("GPSLatReading").innerHTML = GPSLatReading;
}, (errorObject) => {
  console.log('The read failed: ' + errorObject.name);
});

databaseGPSLong.on('value', (snapshot) => {
    GPSLongReading = snapshot.val();
    console.log(GPSLongReading);
    document.getElementById("GPSLongReading").innerHTML = GPSLongReading;
  }, (errorObject) => {
    console.log('The read failed: ' + errorObject.name);
  });

databaseInt.on('value', (snapshot) => {
  intReading = snapshot.val();
  console.log(intReading);
  document.getElementById("reading-int").innerHTML = intReading;
}, (errorObject) => {
  console.log('The read failed: ' + errorObject.name);
});

databaseCam.on('value', (snapshot) => {
    camReading = snapshot.val();
    console.log(camReading);
    document.getElementById("reading-cam").innerHTML = camReading;
  }, (errorObject) => {
    console.log('The read failed: ' + errorObject.name);
  });

  // Reference to Picture Storage Path
var imgRef = storageRef.child('data/photo.jpg');

firebase.auth().signInAnonymously().then(function() {

  imgRef.getDownloadURL().then(function(url){
    // `url` is the download URL for 'data/photo.jpg'
    document.querySelector('img').src = url;
    
  }).catch(function(error) {
    console.error(error);
  });
});

imgRef.getMetadata()
  .then((metadata) => {
    console.log(metadata);
    date = new Date(metadata.timeCreated);
    console.log(date.getFullYear()+'-' + (date.getMonth()+1) + '-'+date.getDate());
    console.log(date.getHours() + ":" + date.getMinutes() + ":" + date.getSeconds());
    var time = (date.getHours() + ":" + date.getMinutes() + ":" + date.getSeconds());
    var writtenDate = (date.getFullYear()+'-' + (date.getMonth()+1) + '-'+date.getDate());
    document.getElementById("date-time").innerHTML = time + " at " + writtenDate;
  })
  .catch((error)=> {
    console.error(error);
  });

  // Database path for GPIO states
var dbCamPathOutput = 'camera/bool';
// Database references
var dbCamRefOutput = firebase.database().ref().child(dbCamPathOutput);

  button.onclick = () =>{
    dbCamRefOutput.set(true);
    setInterval(function(){
        window.location.reload();
    }, 20000);
    
  }

let map;

async function initMap() {
  const { Map } = await google.maps.importLibrary("maps");

  map = new Map(document.getElementById("map"), {
    center: { lat: -34.397, lng: 150.644 },
    zoom: 8,
  });
}

initMap();
