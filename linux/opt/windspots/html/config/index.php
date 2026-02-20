<!DOCTYPE html>
<html lang="en">
<!-- WindS W1234 -->
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
                  <label class="col-xs-4 textRight">Anemometer: </label>
                  <div class="col-xs-2">
                    <label class="switch">
                      <input id="anemoValue" type="checkbox">
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


    <script>
<?php
$version = trim((string) @file_get_contents('/opt/windspots/etc/version'));
$windspots_ini = parse_ini_file('/opt/windspots/etc/main') ?: [];
$initial = [
  'version' => $version,
  'stationValue' => (string)($windspots_ini['STATION'] ?? ''),
  'nameValue' => (string)($windspots_ini['STATION_NAME'] ?? ''),
  'altitudeValue' => (string)($windspots_ini['ALTITUDE'] ?? ''),
  'dirAdjustValue' => (string)($windspots_ini['DIRADJ'] ?? ''),
  'mhzValue' => (($windspots_ini['WS433'] ?? 'N') !== 'N'),
  'anemoValue' => (($windspots_ini['WSANEMO'] ?? 'N') !== 'N'),
  'solarValue' => (($windspots_ini['WSSOLAR'] ?? 'N') !== 'N'),
  'temperatureValue' => (($windspots_ini['WSTEMP'] ?? 'N') !== 'N'),
  'rj45Value' => (($windspots_ini['RJ45'] ?? 'N') !== 'N'),
  'wifiValue' => (($windspots_ini['WIFI'] ?? 'N') !== 'N'),
  'pppValue' => (($windspots_ini['PPP'] ?? 'N') !== 'N'),
  'camRotateValue' => (string)($windspots_ini['CAMROTATE'] ?? '0'),
  'camAdjustValue' => (string)($windspots_ini['CAMADJUST'] ?? ''),
];
echo 'const initialState=' . json_encode($initial, JSON_UNESCAPED_SLASHES) . ';';
?>
const $ = (id) => document.getElementById(id);
const boolToYN = (v) => (v ? 'Y' : 'N');
let nbRefresh = 0;

const UI = {
  modalLoader: $('modalLoader'),
  modalProvider: $('modalProvider'),
  pppSignal: $('pppSignal'),
  imgContainer: $('imgContainer'),
  spotImg: $('spotImg'),
  showRotateValue: $('showRotateValue')
};

UI.modalLoader.style.display = 'none';

function setIndicatorColor(id, ok) {
  const el = $(id);
  const color = ok ? '#1AB188' : '#8A3030';
  el.style.color = color;
  if (id.startsWith('i2c')) el.style.borderColor = color;
}

function updateSignal(iconValue) {
  const classes = ['good', 'ok', 'bad', 'one-bar', 'two-bars', 'three-bars', 'four-bars'];
  UI.pppSignal.classList.remove(...classes);
  const icon = parseInt(iconValue, 10);
  if (icon >= 4) {
    UI.pppSignal.classList.add('good', 'four-bars');
  } else if (icon === 3) {
    UI.pppSignal.classList.add('ok', 'three-bars');
  } else if (icon === 2) {
    UI.pppSignal.classList.add('bad', 'two-bars');
  } else {
    UI.pppSignal.classList.add('bad', 'one-bar');
  }
}

function doRefresh() {
  const xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function () {
    if (this.readyState !== 4) return;
    if (this.status === 200) {
      const alim = JSON.parse(this.responseText);
      $('batteryVolt').textContent = `${alim.batteryVolt} V`;
      $('batteryMhz').textContent = `${alim.batteryMhz} Mhz`;
      $('solarVolt').textContent = `${alim.solarVolt} V`;
      $('solarMhz').textContent = `${alim.solarMhz} Mhz`;
      $('stationVolt').textContent = `${alim.stationVolt} V`;
      $('stationMhz').textContent = `${alim.stationMhz} Mhz`;
      $('stationDir').textContent = `${alim.dir.toFixed(1)}°`;
      $('stationSpeed').textContent = `${alim.speed.toFixed(1)} km/h`;
      $('stationTemp').textContent = `${alim.temp.toFixed(1)}°`;
      $('stationPressure').textContent = `${alim.pressure} hPa`;
      $('stationBTemp').textContent = `${alim.bTemp}°`;
      $('log').textContent = `${alim.log}`;
      setIndicatorColor('i2c40', alim.I2C40 === true);
      setIndicatorColor('i2c41', alim.I2C41 === true);
      setIndicatorColor('i2c43', alim.I2C43 === true);
      setIndicatorColor('i2c48', alim.I2C48 === true);
      setIndicatorColor('i2c77', alim.I2C77 === true);
      setIndicatorColor('rj45', alim.lan === true);
      setIndicatorColor('wifi', alim.wlan === true);
      setIndicatorColor('ppp', alim.ppp === true);
      $('rj45IP').textContent = alim.lanIP;
      $('wifiIP').textContent = alim.wlanIP;
      $('pppIP').textContent = alim.IPADDRESS;
      $('ssidValue').value = alim.ssid;
      $('pppProviderValue').textContent = alim.FULLNAME;
      $('pppWorkMode').textContent = alim.WORKMODE;
      updateSignal(alim.SIGNALICON);

      if (nbRefresh === 0) UI.spotImg.style.backgroundImage = `url(img/${alim.image})`;
      if (++nbRefresh > 14) nbRefresh = 0;
    }
    setTimeout(doRefresh, 2000);
  };
  xhr.open('GET', '/php/infos.php', true);
  xhr.send();
}

function setValue() {
  document.title = `${initialState.stationValue} MWS: ${initialState.version}`;
  $('version').textContent = initialState.version;
  $('stationValue').value = initialState.stationValue;
  $('nameValue').value = initialState.nameValue;
  $('altitudeValue').value = initialState.altitudeValue;
  $('dirAdjustValue').value = initialState.dirAdjustValue;
  $('433Value').checked = initialState.mhzValue;
  $('anemoValue').checked = initialState.anemoValue;
  $('solarValue').checked = initialState.solarValue;
  $('temperatureValue').checked = initialState.temperatureValue;
  $('rj45Value').checked = initialState.rj45Value;
  $('wifiValue').checked = initialState.wifiValue;
  $('pppValue').checked = initialState.pppValue;
  $('camAdjustValue').value = initialState.camAdjustValue;
  $('camRotateValue').textContent = initialState.camRotateValue;
  $('cameraRotationValue').value = 0;

  if (!initialState.solarValue) switchSolar(false);
  doRefresh();
}

function imgRotate(rotation) {
  UI.spotImg.style.transform = `rotate(${rotation}deg)`;
  UI.showRotateValue.textContent = `${rotation}°`;
}

function showGrid(stat) {
  const show = stat === 'show';
  UI.imgContainer.style.outline = show ? '2px solid rgba(255,255,255,0.8)' : 'none';
  document.querySelectorAll('.grid').forEach((el) => {
    el.style.display = show ? 'block' : 'none';
  });
}

function showWifiModal(stat) {
  if (stat === 'show') {
    $('ssid').value = '';
    $('wpa').value = '';
    UI.modalProvider.style.display = 'block';
    return;
  }
  UI.modalProvider.style.display = 'none';
}

function postForm(url, data) {
  const xhr = new XMLHttpRequest();
  xhr.open('POST', url, true);
  xhr.send(data);
}

function doUpdate() {
  const data = new FormData();
  data.append('station', $('stationValue').value);
  data.append('station_name', $('nameValue').value);
  data.append('altitude', $('altitudeValue').value);
  data.append('dir_adj', $('dirAdjustValue').value);
  data.append('camrotate', $('cameraRotationValue').value);
  data.append('cam_adj', $('camAdjustValue').value);
  data.append('r433mhz', boolToYN($('433Value').checked));
  data.append('anemo', boolToYN($('anemoValue').checked));
  data.append('solar', boolToYN($('solarValue').checked));
  data.append('temp', boolToYN($('temperatureValue').checked));
  data.append('rj45', boolToYN($('rj45Value').checked));
  data.append('wifi', boolToYN($('wifiValue').checked));
  data.append('ppp', boolToYN($('pppValue').checked));
  data.append('pppProvider', $('pppProviderValue').value || '');
  data.append('ssid', $('ssidValue').value);
  postForm('update.php', data);

  switchSolar($('solarValue').checked);
  UI.modalLoader.style.display = 'block';
  setTimeout(() => { UI.modalLoader.style.display = 'none'; }, 1000);

  const current = parseInt($('camRotateValue').textContent, 10) || 0;
  const delta = parseInt($('cameraRotationValue').value, 10) || 0;
  $('camRotateValue').textContent = String(current + delta);
}

function doWifi() {
  UI.modalProvider.style.display = 'none';
  const data = new FormData();
  data.append('ssid', $('ssid').value);
  data.append('wpa', $('wpa').value);
  postForm('wifiwpa.php', data);
  UI.modalLoader.style.display = 'block';
  setTimeout(() => { UI.modalLoader.style.display = 'none'; }, 1000);
}

function switchSolar(enabled) {
  const display = enabled ? 'inline-block' : 'none';
  $('i2c40').style.display = display;
  $('i2c41').style.display = display;
  $('i2c43').style.display = display;
  const rowDisplay = enabled ? 'block' : 'none';
  $('batteryRow').style.display = rowDisplay;
  $('solarRow').style.display = rowDisplay;
  $('stationRow').style.display = rowDisplay;
}

function doReboot() {
  const data = new FormData();
  data.append('reboot', new Date().toISOString());
  postForm('reboot.php', data);
  UI.modalLoader.style.display = 'block';
  setTimeout(() => { UI.modalLoader.style.display = 'none'; }, 60000);
}
</script>

</body>
</html>
