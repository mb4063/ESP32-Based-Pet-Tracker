// Complete Project Details at: https://RandomNerdTutorials.com/

// Database Paths
var dataGPSLatPath = 'gps/latitude';
var dataGPSLonPath = 'gps/longitude';
var dataFenceLatPath = 'fence/latitude';
var dataFenceLonPath = 'fence/longitude';
var dataRadiusPath = 'fence/fenceRadius';
var cameraPath = 'camera/bool';

// Get a database reference 
const databaseGPSLat = database.ref(dataGPSLatPath);
const databaseGPSLong = database.ref(dataGPSLonPath);
const databaseFenceLat = database.ref(dataFenceLatPath);
const databaseFenceLong = database.ref(dataFenceLonPath);
const databaseRadius = database.ref(dataRadiusPath);
const databaseCam = database.ref(cameraPath);

// Variables to save database current values
var GPSLatReading;
var GPSLongReading;
var fenceLatReading;
var fenceLongReading;
var radiusReading;
var camReading;

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




databaseCam.on('value', (snapshot) => {
    camReading = snapshot.val();
    console.log(camReading);
    document.getElementById("reading-cam").innerHTML = camReading;
}, (errorObject) => {
    console.log('The read failed: ' + errorObject.name);
});


var dbCamPathOutput = 'camera/bool';

var dbCamRefOutput = firebase.database().ref().child(dbCamPathOutput);

button.onclick = () => {
    dbCamRefOutput.set(true);
    setInterval(function () {
        window.location.reload();
    }, 20000);

}

databaseGPSLong.on('value', (snapshot) => {
    GPSLongReading = snapshot.val();
    databaseGPSLat.on('value', (snapshot) => {
        GPSLatReading = snapshot.val();
        var map = L.map('map').setView([GPSLatReading, GPSLongReading], 13);
        L.tileLayer('https://tile.openstreetmap.org/{z}/{x}/{y}.png', {
            maxZoom: 19,
            attribution: '&copy; <a href="http://www.openstreetmap.org/copyright">OpenStreetMap</a>'
        }).addTo(map);
        var marker = L.marker([GPSLatReading, GPSLongReading]).addTo(map);
        databaseRadius.on('value', (snapshot) => {
            radiusReading = snapshot.val();

            databaseFenceLat.on('value', (snapshot) => {
                fenceLatReading = snapshot.val();
                

            });

            databaseFenceLong.on('value', (snapshot) => {
                fenceLongReading = snapshot.val();

                var circle = L.circle([fenceLatReading, fenceLongReading], {
                    color: 'red',
                    fillColor: '#f03',
                    fillOpacity: 0.5,
                    radius: radiusReading
                }).addTo(map);

                var popup = L.popup();

                function onMapClick(e) {

                    popup
                        .setLatLng(e.latlng)
                        .setContent("You clicked the map at " + e.latlng.toString())
                        .openOn(map);
                    var dbfenceLatPath = 'fence/latitude';
                    var dbfenceLatOutput = firebase.database().ref().child(dbfenceLatPath);
                    dbfenceLatOutput.set(e.latlng.lat);

                    var dbfenceLongPath = 'fence/longitude';
                    var dbfenceLongOutput = firebase.database().ref().child(dbfenceLongPath);
                    dbfenceLongOutput.set(e.latlng.lng);
                }

                map.on('click', onMapClick);



            });

        });
    });

});







