<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="utf-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta name="description" content="">
    <meta http-equiv="cache-control" content="no-cache" />
    <meta name="author" content="Windspot Sàrl">

    <title>MWS</title>
    <link href="../css/wsm.css" rel="stylesheet">
</head>

<body id="page-top" class="index" onload="setValue()">
<!--
    <header>
        <div class="container">
            <div class="row">
                <div class="col-md-12 textCenter">
                    <h1>Management Windspots Station</h1>
                </div>
                <div class="col-md-12 textCenter">
                    Version: <span id="version"></span>
                </div>
            </div>
        </div>
    </header>
-->
    <main>
        <div class="container">
            <div class="col-md-6">

            <div class="row">
                <div class="col-md-12 textCenter">
                  <div class="gridContainer">
                    <span>Version: <span id="version"></span></span>  
                    <span>&nbsp</span>  
                    <span><input style="width:auto; background:#8a3030;" id="stationReboot" type="submit" value="Reboot" onclick="return doReboot()" ></span> 
                  </div>
                </div>
            </div>

                <div class="row">
                    <label class="col-xs-4 textRight">Station: </label>
                    <div class="col-xs-8">
                      <input id="stationValue" type="text" value="" >
                  </div>
                </div>

                <div class="row">
                  <label class="col-xs-4 textRight">Name: </label>
                  <div class="col-xs-8">
                    <input id="nameValue" type="text" value="">
                  </div>
                </div>

                <div class="row">
                  <label class="col-xs-4 textRight">Altitude: </label>
                  <div class="col-xs-8">
                    <input id="altitudeValue" value="" type="text">
                  </div>
                </div>

                <div class="row">
                  <label class="col-xs-4 textRight">Direction adjust: </label>
                  <div class="col-xs-8">
                    <input id="dirAdjustValue" value="" type="text">
                  </div>
                </div>

                
                <div class="row">
                  <label id="rj45" class="col-xs-4 textRight">RJ45: </label>
                  <div class="col-xs-2">
                    <label class="switch">
                      <input id="rj45Value" type="checkbox">
                      <div class="slider round"></div>
                    </label>
                  </div>

                  <label id="rj45IP" class="col-xs-3 textRight"></label>
                </div>

                <div class="row">
                  <label id="wifi" class="col-xs-4 textRight">Wifi: </label>
                  <div class="col-xs-2">
                    <label class="switch">
                      <input id="wifiValue" type="checkbox">
                      <div class="slider round"></div>
                    </label>
                  </div>

                  <label id="wifiIP"class="col-xs-3 textRight">0.0.0.0</label>
                </div>

                <div class="row">
                  <label class="col-xs-4 textRight">SSID: </label>
                  <div class="col-xs-8">
                    <input id="ssidValue" onClick="showWifiModal('show')" type="text" value="" readonly="readonly">
                  </div>
                </div>

                <div class="row">
                  <label id="ppp" class="col-xs-4 textRight">3G: </label>
                  <div class="col-xs-2">
                    <label class="switch">
                      <input id="pppValue" type="checkbox">
                      <div class="slider round"></div>
                    </label>
                  </div>

                  <label id="pppIP"class="col-xs-3 textRight">0.0.0.0</label>
                </div>

                <div class="row">
                  <label class="col-xs-4 textRight"></label>
                  <label class="col-xs-8 textCenter" id="pppProviderValue"></label>
                </div>

                <div class="row">
                  <label id="pppWork" class="col-xs-4 textRight"> </label>
                  <div class="col-xs-4" >
                    <div id="pppSignal" style="height:50px; width:100px;" class="signal-bars mt1 sizing-box">
                      <div class="first-bar bar"></div>
                      <div class="second-bar bar"></div>
                      <div class="third-bar bar"></div>
                      <div class="fourth-bar bar"></div>
                      <div class="fifth-bar bar"></div>
                    </div>
                  </div>
                  <label id="pppWorkMode" class="col-xs-4 textLeft"></label>
                </div>

                <div class="row">
                  <label class="col-xs-4 textRight">433 Mhz: </label>
                  <div class="col-xs-2">
                    <label class="switch">
                      <input id="433Value" type="checkbox">
                      <div class="slider round"></div>
                    </label>
                  </div>
                          
                  <label class="col-xs-3 textRight">Temperature: </label>
                  <div class="col-xs-3">
                    <label class="switch">
                      <input id="temperatureValue" type="checkbox">
                      <div class="slider round"></div>
                    </label>
                  </div>
                </div>

                <div class="row">
                  <label class="col-xs-4 textRight">Anemometer: </label>
                  <div class="col-xs-2">
                    <label class="switch">
                      <input id="anemoValue" type="checkbox">
                      <div class="slider round"></div>
                    </label>
                  </div>

                  <label class="col-xs-3 textRight">Solar </label>
                  <div class="col-xs-3">
                    <label class="switch">
                      <input id="solarValue" type="checkbox">
                      <div class="slider round"></div>
                    </label>
                  </div>
                </div>

              </div>              
              

                <!-- infos -->
            <div class="col-md-6">
                <div class="row">
                  <label class="col-xs-2">i2c: </label>
                  <div class="col-xs-10">
                    <span id="i2c40" class="square iTcStat">40</span>
                    <span id="i2c41" class="square iTcStat">41</span>
                    <span id="i2c43" class="square iTcStat">43</span>
                    <span id="i2c48" class="square iTcStat">48</span>
                    <span id="i2c77" class="square iTcStat">77</span>
                  </div>
                </div>

                <div id="solarRow" class="row">
                  <label class="col-xs-2">Solar[40]:</label>
                  <div class="col-xs-10">
                    <span id="solarVolt" class="square squareVolt"></span>
                    <span id="solarMhz" class="square squareVolt"></span>
                  </div>
                </div>

                <div id="batteryRow" class="row">
                  <label class="col-xs-2">Battery[41]:</label>
                  <div class="col-xs-10">
                    <span id="batteryVolt" class="square squareVolt"></span>
                    <span id="batteryMhz" class="square squareVolt"></span>
                  </div>
                </div>

                <div id="stationRow" class="row">
                  <label class="col-xs-2">Station[43]:</label>
                  <div class="col-xs-10">
                    <span id="stationVolt" class="square squareVolt"></span>
                    <span id="stationMhz" class="square squareVolt"></span>
                  </div>
                </div>

                <div id="temperatureRow" class="row">
                  <label class="col-xs-2">Temp[48:1]:</label>
                  <div class="col-xs-10">
                    <span id="stationTemp" class="square squareVolt"></span>
                  </div>
                </div>

                <div id="bmp280Row" class="row">
                  <label class="col-xs-2">Bmp280[77]:</label>
                  <div class="col-xs-10">
                    <span id="stationPressure" class="square squareVolt"></span>
                    <span id="stationBTemp" class="square squareVolt"></span>
                  </div>
                </div>

                <div id="anemoRow" class="row">
                  <label class="col-xs-2">Anemo[48:0]:</label>
                  <div class="col-xs-10">
                    <span id="stationDir" class="square squareVolt"></span>
                    <span id="stationSpeed" class="square squareVolt"></span>
                  </div>
                </div>

                <div class="row">
                  <div class="col-md-12">
                    <label>Log:</label>
                    <p id="log"></p>
                  </div>
                </div>

              </div>

                <div class="col-md-12 noPad">
                    <div id="imgContainer">
                      <div id="showRotateValue"></div>
                      <div id="spotImg"></div>
                      <div class="gridContainer">
                        <span class="grid vgrid vgrid1"></span>
                        <span class="grid vgrid vgrid2"></span>
                        <span class="grid vgrid vgrid3"></span>
                        <span class="grid hgrid hgrid1"></span>
                        <span class="grid hgrid hgrid2"></span>
                        <span class="grid hgrid hgrid3"></span>
                      </div>
                    </div>               
              </div>
              <div class="col-md-12">
                <div class="row">
                  <input id="cameraRotationValue" onmouseup="showGrid('hide')" onmousedown="showGrid('show')" oninput="imgRotate(this.value)" type="range" min="-10" max="10">
                </div>
              </div>
              
              <div class="col-md-6">
                <div class="row">
                  <label class="col-xs-4 textRight">Camera Rotate: </label>
                  <label id="camRotateValue"class="col-xs-2 textRight">0</label>
                </div>

                <div class="row">
                  <label class="col-xs-4 textRight">Camera Height Adj.(0..650): </label>
                  <div class="col-xs-8">
                    <input id="camAdjustValue" value="" type="text">
                  </div>
                </div>
              </div>
              
              <div id="modalProvider" tabindex="-1" >
                <div class="modalFade"></div>
                <div id="providerBox" >
                  <svg class="crossClose" onClick="showWifiModal('hide')" width="40" height="40" viewBox="0 0 40 40">
                    <line x1="0" y1="0" x2="40" y2="40"/>
                    <line x1="0" y1="40" x2="40" y2="0"/>
                  </svg>
                  <div class="row">
                      <label class="col-xs-12">SSID: </label>
                      <div class="col-xs-12">
                        <input id="ssid" type="text">
                      </div>
                  </div>
                  <div class="row">
                      <label class="col-xs-12">WPA Key:</label>
                      <div class="col-xs-12">
                        <input id="wpa" type="text" >
                      </div>
                  </div>
                  <div class="row">
                    <div class="col-xs-12">
                       <button class="ssIdSave" type="button" onClick="doWifi()">Save</button>
                    </div>
                  </div>
                </div>
              </div>

              <div id="modalLoader" tabindex="-1" >
                <div class="modalFade"></div>
                <div id="loadderBox" >
                  <svg id="loader" width="100" height="100">
                    <circle class="loadC1" cx="50" cy="10" r="5" />
                    <circle class="loadC2" cx="65.3" cy="13.04" r="5" />
                    <circle class="loadC3" cx="78.28" cy="21.71" r="5" />
                    <circle class="loadC4" cx="86.92" cy="34.69" r="5" />
                    <circle class="loadC5" cx="90" cy="50" r="5" />
                    <circle class="loadC6" cx="86.95" cy="65.3" r="5" />
                    <circle class="loadC7" cx="78.28" cy="78.28" r="5" />
                    <circle class="loadC8" cx="65.03" cy="86.95" r="5" />
                    <circle class="loadC9" cx="50" cy="90" r="5" />
                    <circle class="loadC10" cx="34.69" cy="86.95" r="5" />
                    <circle class="loadC11" cx="21.71" cy="78.28" r="5" />
                    <circle class="loadC12" cx="13.04" cy="65.3" r="5" />
                    <circle class="loadC13" cx="10" cy="50" r="5" />
                    <circle class="loadC14" cx="13.04" cy="34.69" r="5" />
                    <circle class="loadC15" cx="21.71" cy="21.71" r="5" />
                    <circle class="loadC16" cx="34.69" cy="13.04" r="5" />
                  </svg>
                </div>
              </div>

                
        </div>
    </main>

    <footer>
      <div class="container">
        <button class="apllyBtn" type="submit" onclick="return doUpdate()">Apply</button>
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
    var nameValue="<?php echo $windspots_ini['STATION_NAME'];?>";
    var pppProviderValue="uneCleUltraSecure";
    var altitudeValue="<?php echo $windspots_ini['ALTITUDE'];?>";
    var dirAdjustValue="<?php echo $windspots_ini['DIRADJ'];?>";;
    var mhzValue=<?php if(strcmp($windspots_ini['WS433'],"N")) { echo 'true'; } else { echo 'false'; } ?>;
    var anemoValue=<?php if(strcmp($windspots_ini['WSANEMO'],"N")) { echo 'true'; } else { echo 'false'; } ?>;
    var solarValue=<?php if(strcmp($windspots_ini['WSSOLAR'],"N")) { echo 'true'; } else { echo 'false'; } ?>;
    var temperatureValue=<?php if(strcmp($windspots_ini['WSTEMP'],"N")) { echo 'true'; } else { echo 'false'; } ?>;
    var rj45Value=<?php if(strcmp($windspots_ini['RJ45'],"N")) { echo 'true'; } else { echo 'false'; } ?>;
    var wifiValue=<?php if(strcmp($windspots_ini['WIFI'],"N")) { echo 'true'; } else { echo 'false'; } ?>;
    var pppValue=<?php if(strcmp($windspots_ini['PPP'],"N")) { echo 'true'; } else { echo 'false'; } ?>;
    var pppProvider="";
    var camRotateValue="<?php echo $windspots_ini['CAMROTATE'];?>";
    var camAdjustValue="<?php echo $windspots_ini['CAMADJUST'];?>";
    
    var ssidValue="";
    var connexionStat=true;
    var batteryVolt=0;
    var batteryMhz=0;
    var solarVolt=0;
    var solarMhz=0;
    var stationVolt=0;
    var stationMhz=0;
    var log="";
    var nbRefresh = 0;

    document.getElementById('modalLoader').style.display="none";
    if(solarValue==false) switchSolar(false);
    
    function doRefresh() {
      var xmlhttp = new XMLHttpRequest();
      xmlhttp.onreadystatechange = function() {
          if (this.readyState == 4 && this.status == 200) {
            var alim = JSON.parse(this.responseText);
            console.log(alim);
            document.getElementById('batteryVolt').innerHTML=alim.BATTERYVOLTAGE+' V'; 
            document.getElementById('batteryMhz').innerHTML=Math.round(alim.BATTERYPOWER)+' mW';
            document.getElementById('solarVolt').innerHTML=alim.SOLARVOLTAGE+' V';
            document.getElementById('solarMhz').innerHTML=Math.round(alim.SOLARPOWER)+' mW';
            document.getElementById('stationVolt').innerHTML=alim.LOADVOLTAGE+' V';
            document.getElementById('stationMhz').innerHTML=Math.round(alim.LOADPOWER)+' mW';
            document.getElementById('stationTemp').innerHTML=Math.round(alim.TEMPERATURE)+' °';
            document.getElementById('stationPressure').innerHTML=Math.round(alim.SEALEVEL)+'/'+Math.round(alim.PRESSURE);
            document.getElementById('stationBTemp').innerHTML=Math.round(alim.INBOXTEMP)+' °';
            document.getElementById('stationDir').innerHTML=Math.round(alim.DIRECTION)+' °';
            document.getElementById('stationSpeed').innerHTML=Math.round(alim.SPEED)+' m/s';
            document.getElementById('log').innerHTML=alim.LOG;
            var i2c=document.getElementById('i2c40');
            var i2cValue = alim.I2C40;
            if(i2cValue==true){ i2c.style.color="#1AB188"; i2c.style.borderColor="#1AB188"; } else { i2c.style.color="#8A3030"; i2c.style.borderColor="#8A3030"; }
            i2c=document.getElementById('i2c41'); 
            i2cValue = alim.I2C41;
            if(i2cValue==true){ i2c.style.color="#1AB188"; i2c.style.borderColor="#1AB188"; } else { i2c.style.color="#8A3030"; i2c.style.borderColor="#8A3030"; }
            i2c=document.getElementById('i2c43'); 
            i2cValue = alim.I2C43;
            if(i2cValue==true){ i2c.style.color="#1AB188"; i2c.style.borderColor="#1AB188"; } else { i2c.style.color="#8A3030"; i2c.style.borderColor="#8A3030"; }
            i2c=document.getElementById('i2c48'); 
            i2cValue = alim.I2C48;
            if(i2cValue==true){ i2c.style.color="#1AB188"; i2c.style.borderColor="#1AB188"; } else { i2c.style.color="#8A3030"; i2c.style.borderColor="#8A3030"; }
            i2c=document.getElementById('i2c77'); 
            i2cValue = alim.I2C77;
            if(i2cValue==true){ i2c.style.color="#1AB188"; i2c.style.borderColor="#1AB188"; } else { i2c.style.color="#8A3030"; i2c.style.borderColor="#8A3030"; }
            var netlab=document.getElementById('rj45'); 
            var network = alim.lan;
            if(network==true) { netlab.style.color="#1AB188"; } else { netlab.style.color="#8A3030";}
            document.getElementById('rj45IP').innerHTML=alim.lanIP;
            netlab=document.getElementById('wifi'); 
            network = alim.wlan;
            if(network==true) { netlab.style.color="#1AB188"; } else { netlab.style.color="#8A3030";}
            document.getElementById('wifiIP').innerHTML=alim.wlanIP;
            netlab=document.getElementById('ppp'); 
            network = alim.ppp;
            if(network==true) { netlab.style.color="#1AB188"; } else { netlab.style.color="#8A3030";}
            document.getElementById('pppIP').innerHTML=alim.IPADDRESS;
            document.getElementById('ssidValue').value=alim.ssid;
              // hilink
            document.getElementById('pppProviderValue').innerHTML=alim.FULLNAME;  
            document.getElementById('pppWorkMode').innerHTML=alim.WORKMODE;
            //  good four-bars
            document.getElementById('pppSignal').classList.remove('good');
            document.getElementById('pppSignal').classList.remove('ok');
            document.getElementById('pppSignal').classList.remove('bad');
            document.getElementById('pppSignal').classList.remove('one-bar');
            document.getElementById('pppSignal').classList.remove('two-bars');
            document.getElementById('pppSignal').classList.remove('three-bars');
            document.getElementById('pppSignal').classList.remove('four-bars');
            switch(parseInt(alim.SIGNALICON)) {
              case 5:
              case 4:
                document.getElementById('pppSignal').classList.add('good');
                document.getElementById('pppSignal').classList.add('four-bars');
                break;
              case 3:
                document.getElementById('pppSignal').classList.add('ok');
                document.getElementById('pppSignal').classList.add('three-bars');
                break;
              case 2:
                document.getElementById('pppSignal').classList.add('bad');
                document.getElementById('pppSignal').classList.add('two-bars');
                break;
              default:
                document.getElementById('pppSignal').classList.add('bad');
                document.getElementById('pppSignal').classList.add('one-bar');
                break;
            }
              // reload image
            var bgImage=document.getElementById('spotImg');
            var seconds = new Date().getTime() / 1000;
            if(nbRefresh == 0) bgImage.style.backgroundImage = "url(/img/"+alim.image+")";
            if(nbRefresh++ > 14) nbRefresh = 0;
            setTimeout(doRefresh, 2000);
          }
      };
      xmlhttp.open("GET", "/php/infos.php", true);
      xmlhttp.send();
    }

    function setValue(){ 
        // reset tous les champs 
        for(i=0;i<document.getElementsByTagName('input').length;i++){
          field=document.getElementsByTagName('input')[i];
          fieldType=field.getAttribute('type');
          if (fieldType=="text" || fieldType=="password"){
            field.value="";
          }else if(fieldType=="checkbox"){
            field.checked=false;
          }else if(fieldType=="range"){
            field.value=0;
          }
        }
        
        document.getElementsByTagName('title')[0].innerHTML=stationValue+' MWS: '+version;
        document.getElementById('version').innerHTML=version;
        document.getElementById('stationValue').value=stationValue;
        document.getElementById('nameValue').value=nameValue;
        document.getElementById('pppProviderValue').value=pppProviderValue;
        document.getElementById('altitudeValue').value=altitudeValue;
        document.getElementById('dirAdjustValue').value=dirAdjustValue;
        document.getElementById('433Value').checked=mhzValue;
        document.getElementById('anemoValue').checked=anemoValue;
        document.getElementById('solarValue').checked=solarValue;
        document.getElementById('temperatureValue').checked=temperatureValue;
        document.getElementById('rj45Value').checked=rj45Value;
        document.getElementById('wifiValue').checked=wifiValue;
        document.getElementById('pppValue').checked=pppValue;
        // document.getElementById('pppProviderValue').value=pppProvider;
        document.getElementById('camAdjustValue').value=camAdjustValue;
        document.getElementById('camRotateValue').innerHTML=camRotateValue;
        
        doRefresh(); 
    }

    function imgRotate(cameraRotationValue){
      var img =document.getElementById('spotImg');
      var showRotateValue=document.getElementById('showRotateValue');
      console.log(cameraRotationValue);
      img.style.transform='rotate('+cameraRotationValue+'deg)';
      showRotateValue.innerHTML=cameraRotationValue+"�";
    }
    
    function showGrid(stat){
      var showRotateValue=document.getElementById('showRotateValue');
      grid=document.getElementsByClassName("grid");
      //showRotateValue.style.color="#1AB188";
      if (stat=='show'){
        document.getElementById('imgContainer').style.outline="2px solid rgba(255,255,255,0.8)";
        for(i=0;i<grid.length;i++){
          grid[i].style.display="block";
        } 
      }else{
        for(i=0;i<grid.length;i++){
          document.getElementById('imgContainer').style.outline="none";
          grid[i].style.display="none";
        } 
      }
    }
    
    function showWifiModal(stat){
      if(stat=='show'){
        document.getElementById('ssid').value="";
        document.getElementById('wpa').value="";
        document.getElementById('modalProvider').style.display="block";
      }else{
        document.getElementById('modalProvider').style.display="none";
      }
    }
    function sleep(miliseconds) {
       var currentTime = new Date().getTime();
       while (currentTime + miliseconds >= new Date().getTime()) { }
    }
    
    function doUpdate(){
      var station =document.getElementById('stationValue').value;
      var station_name = document.getElementById('nameValue').value;
      var altitude = document.getElementById('altitudeValue').value;
      var dir_adj = document.getElementById('dirAdjustValue').value;
      var cam_rotate = document.getElementById('cameraRotationValue').value;
      var cam_adj = document.getElementById('camAdjustValue').value;
      var r433mhz = "N";
      if(document.getElementById('433Value').checked)
        r433mhz = "Y";
      var anemo = "N";
      if(document.getElementById('anemoValue').checked)
        anemo = "Y";
      var solar = "N";
      if(document.getElementById('solarValue').checked)
        solar = "Y";
      var temp = "N";
      if(document.getElementById('temperatureValue').checked)
        temp = "Y";
      var rj45 = "N";
      if(document.getElementById('rj45Value').checked)
        rj45 = "Y";
      var wifi = "N";
      if(document.getElementById('wifiValue').checked)
        wifi = "Y";
      var ppp = "N";
      if(document.getElementById('pppValue').checked)
        ppp = "Y";
      var ssidValue = document.getElementById('ssidValue').value;
      var pppProvider =document.getElementById('pppProviderValue').value;
      
      var data = new FormData();
      data.append('station', station);
      data.append('station_name', station_name);
      data.append('altitude', altitude);
      data.append('dir_adj', dir_adj);
      data.append('r433mhz', r433mhz);
      data.append('camrotate', cam_rotate);
      data.append('cam_adj', cam_adj);
      data.append('temp', temp);
      data.append('anemo', anemo);
      data.append('solar', solar);
      data.append('rj45', rj45);
      data.append('wifi', wifi);
      data.append('ppp', ppp);
      data.append('pppProvider', pppProvider);
      data.append('ssid', ssidValue);
      
      var xhr = new XMLHttpRequest();
      xhr.open('POST', 'update.php', true);
      xhr.onload = function () {
          // do something to response
          console.log(this.responseText);
      };
      xhr.send(data);  
      // console.log("doUpdate() send");
      var solarNewValue= document.getElementById('solarValue').checked;
      switchSolar(solarNewValue);
      // display loader
      window.setTimeout(function() { document.getElementById('modalLoader').style.display="none"; }, 1000);
      document.getElementById('modalLoader').style.display="block";
      // update camera rotate value
      camRotateValue = parseInt(camRotateValue,10) + parseInt(cam_rotate,10);
      document.getElementById('camRotateValue').innerHTML=camRotateValue;
    }
    
    function doWifi(){
      document.getElementById('modalProvider').style.display="none";
      var ssid = document.getElementById('ssid').value;
      var wpa  = document.getElementById('wpa').value;
      
      var data = new FormData();
      data.append('ssid', ssid);
      data.append('wpa', wpa);
      console.log("ssid:"+ssid+", wpa:"+wpa);
      
      var xhr = new XMLHttpRequest();
      xhr.open('POST', 'wifiwpa.php', true);
      xhr.onload = function () {
          // do something to response
          console.log(this.responseText);
      };
      xhr.send(data);  
      // console.log("updated.") ;
      // Display loader
      window.setTimeout(function() { document.getElementById('modalLoader').style.display="none"; }, 1000);
      document.getElementById('modalLoader').style.display="block";
    }
    
    function switchSolar(bSolar) {
      if(bSolar) {
        document.getElementById('i2c40').style.display="inline-block";
        document.getElementById('i2c41').style.display="inline-block";
        document.getElementById('i2c43').style.display="inline-block";
        document.getElementById('batteryRow').style.display="block";
        document.getElementById('solarRow').style.display="block";
        document.getElementById('stationRow').style.display="block";
      }else{
        document.getElementById('i2c40').style.display="none";
        document.getElementById('i2c41').style.display="none";
        document.getElementById('i2c43').style.display="none";
        document.getElementById('batteryRow').style.display="none";
        document.getElementById('solarRow').style.display="none";
        document.getElementById('stationRow').style.display="none";
      }
    }
    
    function doReboot() {
      var data = new FormData();
      var currentDate = new Date();
      data.append('reboot', currentDate);
      console.log("reboot:"+currentDate);
      
      var xhr = new XMLHttpRequest();
      xhr.open('POST', 'reboot.php', true);
      xhr.onload = function () {
          // do something to response
          console.log(this.responseText);
      };
      xhr.send(data);  
      
      // Display loader
      window.setTimeout(function() { document.getElementById('modalLoader').style.display="none"; }, 60000);
      document.getElementById('modalLoader').style.display="block";
    }
    </script>

</body>

</html>