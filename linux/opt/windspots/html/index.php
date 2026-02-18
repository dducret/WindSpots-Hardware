<!DOCTYPE html>
<html lang="en">

<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta name="apple-mobile-web-app-capable" content="yes">
  <meta http-equiv="cache-control" content="no-cache">
  <title>WS</title>
  <link rel="stylesheet" href="css/ws.css">
</head>

<body>
<?php
  $version = trim((string) shell_exec('cat /opt/windspots/etc/version'));
  $windspots_ini = parse_ini_file('/opt/windspots/etc/main') ?: [];
  $stationValue = $windspots_ini['STATION'] ?? '';
  $stationName = $windspots_ini['STATION_NAME'] ?? '';
?>

<main class="container">
  <header class="row header-row">
    <div class="col col-4 text-right">
      <img id="logo" src="svg/windspots.svg" alt="Windspots logo" width="240" height="80">
    </div>
    <div class="col col-8 text-left">
      <h1 id="stationName"></h1>
    </div>
  </header>

  <section class="row">
    <div class="col col-12" id="imgContainer">
      <img id="spotImg" alt="Spot camera" width="1280" height="307" decoding="async" fetchpriority="high">
    </div>
  </section>

  <section class="row status-row">
    <div class="col col-4" id="petitePatateContainer">
      <svg id="laPetitePatate" viewBox="0 0 300 300" role="img" aria-label="Wind direction">
        <circle cx="150" cy="150" r="100"/>
        <circle cx="150" cy="150" r="110"/>
        <line x1="150" y1="40" x2="150" y2="260"/>
        <line x1="40" y1="150" x2="260" y2="150"/>
        <line x1="79.28" y1="79.28" x2="220.71" y2="220.71"/>
        <line x1="79.28" y1="220.71" x2="220.71" y2="79.28"/>
        <polygon id="windArrow" points="150,50 210,230 150,200 90,230"/>
        <text class="txtBold" x="150" y="20" text-anchor="middle">N</text>
        <text class="txtLight" x="250" y="60" text-anchor="middle">NE</text>
        <text class="txtBold" x="280" y="150" text-anchor="middle">E</text>
        <text class="txtLight" x="250" y="250" text-anchor="middle">SE</text>
        <text class="txtBold" x="150" y="300" text-anchor="middle">S</text>
        <text class="txtLight" x="45" y="250" text-anchor="middle">SO</text>
        <text class="txtBold" x="20" y="150" text-anchor="middle">O</text>
        <text class="txtLight" x="45" y="60" text-anchor="middle">NO</text>
      </svg>
    </div>

    <div class="col col-4 text-center" id="timeBlock">
      <div class="time-row">
        <div id="hour">--:--</div>
        <div id="date">--</div>
      </div>
    </div>

    <div class="col col-4" id="infosSpot">
      <div class="kv"><span>Direction:</span><span id="windDirection"></span></div>
      <div class="kv"><span>Gust:</span><span id="rafale"></span></div>
      <div class="kv"><span>Temperature:</span><span id="temperature"></span></div>
      <div class="kv"><span>Humidity:</span><span id="humidity"></span></div>
      <div class="kv"><span>Pressure:</span><span id="nmMb"></span></div>
    </div>
  </section>
</main>

<footer class="container footer-row">
  <div class="row">
    <div class="col col-6 copyright">Copyright &copy; Windspots 2026</div>
    <div class="col col-6 footerVersion" id="version"></div>
  </div>
</footer>

<script>
(() => {
  'use strict';

  const app = {
    version: <?php echo json_encode($version, JSON_UNESCAPED_UNICODE); ?>,
    stationValue: <?php echo json_encode($stationValue, JSON_UNESCAPED_UNICODE); ?>,
    stationName: <?php echo json_encode($stationName, JSON_UNESCAPED_UNICODE); ?>,
    girouetteOn: true,
    imageTick: 0,
    lastImage: ''
  };

  const ui = {
    title: document.querySelector('title'),
    stationName: document.getElementById('stationName'),
    windArrow: document.getElementById('windArrow'),
    windDirection: document.getElementById('windDirection'),
    rafale: document.getElementById('rafale'),
    humidity: document.getElementById('humidity'),
    temperature: document.getElementById('temperature'),
    nmMb: document.getElementById('nmMb'),
    hour: document.getElementById('hour'),
    date: document.getElementById('date'),
    version: document.getElementById('version'),
    spotImg: document.getElementById('spotImg')
  };

  const setText = (el, text) => {
    if (el.textContent !== text) el.textContent = text;
  };

  const rotateArrow = (direction) => {
    const deg = app.girouetteOn ? direction : 0;
    ui.windArrow.style.transform = `rotate(${deg}deg)`;
    ui.windArrow.style.fill = app.girouetteOn ? '#1AB188' : '#8a3030';
  };

  const updateImage = (imageName) => {
    if (!imageName) return;
    if (app.imageTick === 0 || imageName !== app.lastImage) {
      ui.spotImg.classList.remove('is-visible');
      ui.spotImg.src = `img/${imageName}?t=${Date.now()}`;
      app.lastImage = imageName;
    }
    app.imageTick = (app.imageTick + 1) % 15;
  };

  ui.spotImg.addEventListener('load', () => ui.spotImg.classList.add('is-visible'));

  const doRefresh = async () => {
    try {
      const response = await fetch(`php/weather.php?t=${Date.now()}`, { cache: 'no-store' });
      if (!response.ok) throw new Error(`HTTP ${response.status}`);

      const weather = await response.json();
      const direction = Number(weather.dir) || 0;
      const speed = Number(weather.speed) || 0;

      setText(ui.hour, weather.date || '--:--');
      setText(ui.date, weather.time || '--');
      setText(ui.windDirection, `${direction}°`);
      setText(ui.rafale, `${(speed * 3.6).toFixed(1)} Km/h`);

      if (weather.humidity) setText(ui.humidity, `${weather.humidity}%`);
      if (weather.temperature) setText(ui.temperature, `${weather.temperature}°C`);
      if (weather.barometer) setText(ui.nmMb, `${weather.barometer} mb`);

      rotateArrow(direction);
      updateImage(weather.image);
    } catch (_error) {
      // Keep UI responsive and retry silently.
    } finally {
      setTimeout(doRefresh, 5000);
    }
    };

  ui.title.textContent = `WS: ${app.stationValue}`;
  setText(ui.stationName, app.stationName);
  setText(ui.windDirection, '0°');
  setText(ui.humidity, '0%');
  setText(ui.temperature, '0°C');
  setText(ui.rafale, '0.0 Km/h');
  setText(ui.nmMb, '0 mb');
  setText(ui.version, `Version: ${app.version}`);
  rotateArrow(0);
  doRefresh();
})();
</script>
</body>
</html>
