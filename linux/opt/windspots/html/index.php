<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="utf-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta name="apple-mobile-web-app-capable" content="yes">
    <meta name="description" content="">
    <meta http-equiv="cache-control" content="no-cache" />
    <meta name="author" content="Windspot Sàrl">

    <title>WS</title>
    <link href="css/ws.css" rel="stylesheet">
</head>

<body id="page-top" class="index" onload="setValue()">


    <main>
      <div class="container">

        <div class="row" class="col-md-4">
          <div class="col-xs-4 textRight">
            <img id="logo" src="svg/windspots.svg" style="height: 8vw; width:100%"/>
          </div>
          <div class="col-md-8 textLeft" style="margin-top:4vw;">
            <div id="stationName"></div>
          </div>
        </div>

        <div class="row">
          <div class="col-md-12">
             <div id="imgContainer">
                <div id="spotImg"></div>
              </div>               
          </div>
        </div>

        <div class="row">

          <div id="petitePatateContainer" class="col-md-4">
            <svg style="height: 20vw" id="laPetitePatate" width="300" height="300" viewbox="0 0 300 300">
              <circle cx="150" cy="150" r="100" />
              <circle cx="150" cy="150" r="110" />
              <line x1="150" y1="40" x2="150" y2="260"/>
              <line x1="40" y1="150" x2="260" y2="150"/>
              <line x1="79.28" y1="79.28" x2="220.71" y2="220.71"/>
              <line x1="79.28" y1="220.71" x2="220.71" y2="79.28"/>
              <polygon id="windArrow" points="150,50 210,230 150,200 90,230"/>
              <text class="txtBold"  x="150" y="20"  fill="red" text-anchor="middle">N</text>
              <text class="txtLight" x="250" y="60"  fill="red" text-anchor="middle">NE</text>
              <text class="txtBold"  x="280" y="150" fill="red" text-anchor="middle">E</text>
              <text class="txtLight" x="250" y="250" fill="red" text-anchor="middle">SE</text>
              <text class="txtBold"  x="150" y="300" fill="red" text-anchor="middle">S</text>
              <text class="txtLight" x="45"  y="250" fill="red" text-anchor="middle">SO</text>
              <text class="txtBold"  x="20"  y="150" fill="red" text-anchor="middle">O</text>
              <text class="txtLight" x="45"  y="60"  fill="red" text-anchor="middle">NO</text>
            </svg>
          </div>

          <div class="col-md-4 textCenter">
            <div class="innerRow">
              <div id="hour">19:19</div>
            </div>
            <div class="innerRow">
              <div id="date">12 juin 2017</div>
            </div>
          </div>
              
          <div id="infosSpot" class="col-md-4 textCenter">
            <div class="innerRow">
              <div class="col-xs-4 textRight">
                Direction:
              </div>
              <div class="col-xs-8">
                <span id="windDirection"></span>
              </div>
            </div>

            <div class="innerRow">
              <div class="col-xs-4 textRight">
                Gust:
              </div>
              <div class="col-xs-8">
                <span id="rafale"></span>
              </div>
            </div>

            <div class="innerRow">
              <div class="col-xs-4 textRight">
                &nbsp;
              </div>
              <div class="col-xs-8">
                <span></span>
              </div>
            </div>

              <div class="innerRow">
                <div class="col-xs-4 textRight">
                  Temperature:
                </div>
                <div class="col-xs-8  textLeft">
                  <span id="temperature"></span>
                </div>
              </div>

              <div class="innerRow">
                <div class="col-xs-4 textRight">
                  Humidity:
                </div>
                <div class="col-xs-8 textLeft">
                  <span id="humidity"></span>
                </div>
              </div>

              <div class="innerRow">
                <div class="col-xs-4 textRight">
                  Pressure:
                </div>
                <div class="col-xs-8 textLeft">
                  <span id="nmMb"></span>
                </div>
              </div>

          </div>

        </div>
      </div>
    </main>

    <footer>
       <div class="container">
        <div class="row">
          <div class="col-md-6 copyright">
            Copyright &copy; Windspots 2018
          </div>
          <div class="col-md-6 ">
            <div class="footerVersion" id="version"></div>
          </div>
        </div>
      </div>
    </footer>

    <script type="text/javascript">
    // Les variables
<?php
    $version = shell_exec("cat /opt/windspots/etc/version");
    $windspots_ini = parse_ini_file("/opt/windspots/etc/main");
    $version = str_replace(array("\n", "\r"), '', $version);
?>
    var version="<?php echo $version;?>";
    var stationValue="<?php echo $windspots_ini['STATION'];?>";
    var stationName="<?php echo $windspots_ini['STATION_NAME'];?>";
    
    var girouetteOn=true;

    var windDirection=110;
    var humidity=11;
    var temperature=30;
    var rafale=15;
    var nmMb=1006;

    var nbRefresh = 0;

    function doRefresh() {
      var xmlhttp = new XMLHttpRequest();
      try {
        xmlhttp.onreadystatechange = function() {
          if (this.readyState == 4 && this.status == 200) {
            var weather = JSON.parse(this.responseText);
            console.log(weather);
              // date heure
            document.getElementById('hour').innerHTML=weather.date;
            document.getElementById('date').innerHTML=weather.time;
              // weather date
            // weather.sensor;
            // weather.speed_average;
            if(weather.dir!=0) {
              var windArrow=document.getElementById('windArrow');
              if(girouetteOn==true){
                windArrow.setAttribute('style','fill:#1AB188;-ms-transform: rotate('+weather.dir+'deg);-webkit-transform: rotate('+weather.dir+'deg); transform: rotate('+weather.dir+'deg);');
              } else {
                windArrow.setAttribute('style','fill:#8a3030;-ms-transform: rotate(deg);-webkit-transform: rotate(0deg); transform: rotate(0deg);');
              }
              document.getElementById('windDirection').innerHTML=weather.dir+'&deg;';
              document.getElementById('rafale').innerHTML=weather.speed+' Km/h';
            }
            if(weather.humidity != 0)
              document.getElementById('humidity').innerHTML=weather.humidity+'%';
            if(weather.temperature != 0)
              document.getElementById('temperature').innerHTML=weather.temperature+'&deg;C';
            if(weather.barometer != 0)
              document.getElementById('nmMb').innerHTML=weather.barometer+' mb';
          
              // reload image
            var bgImage=document.getElementById('spotImg');
            if(nbRefresh == 0) bgImage.style.backgroundImage = "url(img/"+weather.image+")";
            if(nbRefresh++ > 14) nbRefresh = 0;
            setTimeout(doRefresh, 5000);
          }
        };
        xmlhttp.open("GET", "php/weather.php", true);
        xmlhttp.send();
      } catch(e) {
        console.log('catch', e);
      }
    }

    function setValue(){ 
        var windArrow=document.getElementById('windArrow');
        if(girouetteOn==true){
          windArrow.setAttribute('style','fill:#1AB188;-ms-transform: rotate('+windDirection+'deg);-webkit-transform: rotate('+windDirection+'deg); transform: rotate('+windDirection+'deg);');
        }else{
          windArrow.setAttribute('style','fill:#8a3030;-ms-transform: rotate(deg);-webkit-transform: rotate(0deg); transform: rotate(0deg);');
        }
        document.getElementsByTagName('title')[0].innerHTML='WS: '+stationValue;
        document.getElementById('stationName').innerHTML=stationName;
        document.getElementById('windDirection').innerHTML=windDirection+'&deg;';
        document.getElementById('humidity').innerHTML=humidity+'%';
        document.getElementById('temperature').innerHTML=temperature+'&deg;C';
        document.getElementById('rafale').innerHTML=rafale+'Km/h';
        document.getElementById('nmMb').innerHTML=nmMb+'Mb';
        document.getElementById('version').innerHTML="Version: "+version;
        
        doRefresh(); 
    }
    </script>

</body>

</html>
