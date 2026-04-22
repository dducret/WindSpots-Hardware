<?php
declare(strict_types=1);

date_default_timezone_set('Europe/Zurich');

$rootPath = str_replace('\\', '/', realpath(__DIR__ . '/../') ?: __DIR__);
$sqliteDb = '/data/sqlite/station_report.db';
$captureDir = '/data/sites/www/data/capture';
$pageVersion = '2.0';

function h(?string $value): string
{
    return htmlspecialchars((string) $value, ENT_QUOTES, 'UTF-8');
}

function parseTimestamp(?string $value): ?int
{
    $value = trim((string) $value);
    if ($value === '' || $value === '0') {
        return null;
    }

    if (preg_match('/^\d{14}$/', $value) !== 1) {
        return null;
    }

    $date = DateTimeImmutable::createFromFormat('YmdHis', $value);
    return $date ? $date->getTimestamp() : null;
}

function formatAge(?int $seconds): string
{
    if ($seconds === null || $seconds < 0) {
        return 'n/a';
    }

    if ($seconds < 60) {
        return $seconds . ' s';
    }

    $minutes = intdiv($seconds, 60);
    if ($minutes < 60) {
        return $minutes . ' min';
    }

    $hours = intdiv($minutes, 60);
    $remainingMinutes = $minutes % 60;
    if ($hours < 24) {
        return $hours . ' h' . ($remainingMinutes > 0 ? ' ' . $remainingMinutes . ' min' : '');
    }

    $days = intdiv($hours, 24);
    $remainingHours = $hours % 24;
    return $days . ' j' . ($remainingHours > 0 ? ' ' . $remainingHours . ' h' : '');
}

function formatClock(?int $timestamp): string
{
    return $timestamp ? date('H:i', $timestamp) : '--:--';
}

function formatDay(?int $timestamp): string
{
    return $timestamp ? date('d/m', $timestamp) : '--/--';
}

function toFloatOrNull($value): ?float
{
    if ($value === null || $value === '') {
        return null;
    }

    if (!is_numeric($value)) {
        return null;
    }

    return (float) $value;
}

function toIntOrNull($value): ?int
{
    if ($value === null || $value === '') {
        return null;
    }

    if (!is_numeric($value)) {
        return null;
    }

    return (int) round((float) $value);
}

function windCompass(?float $direction): string
{
    if ($direction === null) {
        return '-';
    }

    $compass = [
        'N', 'NNE', 'NE', 'ENE',
        'E', 'ESE', 'SE', 'SSE',
        'S', 'SSW', 'SW', 'WSW',
        'W', 'WNW', 'NW', 'NNW',
    ];

    return $compass[((int) round($direction / 22.5)) % 16];
}

function stationImageUrl(string $stationCode): string
{
    return 'https://windspots.org/images.php?imagedir=capture&image=' . rawurlencode($stationCode . '1.jpg') . '&uid=&text=Fermer';
}

function gaugeClass(string $level): string
{
    return 'gauge gauge-' . $level;
}

function stationPrefixRank(string $stationCode): int
{
    foreach (['CHGE', 'CHVD', 'CHVS'] as $index => $prefix) {
        if (strncmp($stationCode, $prefix, strlen($prefix)) === 0) {
            return $index;
        }
    }

    return 99;
}

function stationSeverity(array $station): int
{
    if ($station['station_state'] === 'offline') {
        return 4;
    }

    if ($station['camera_state'] === 'error' || $station['battery_state'] === 'error') {
        return 3;
    }

    if ($station['station_state'] === 'warning' || $station['weather_state'] !== 'ok' || $station['camera_state'] === 'warning') {
        return 2;
    }

    if ($station['summary_state'] !== 'ok') {
        return 1;
    }

    return 0;
}

function summarizeStation(array $row, string $captureDir): array
{
    $stationCode = (string) ($row[0] ?? '');
    $stationName = (string) ($row[1] ?? '');
    $uploadTs = parseTimestamp((string) ($row[2] ?? ''));
    $weatherRaw = trim((string) ($row[3] ?? ''));
    $weatherTs = parseTimestamp($weatherRaw);
    $windDir = toFloatOrNull($row[4] ?? null);
    $windGustKt = (($gustMs = toFloatOrNull($row[5] ?? null)) !== null) ? (int) round($gustMs * 1.943844) : null;
    $windAvgKt = (($avgMs = toFloatOrNull($row[6] ?? null)) !== null) ? (int) round($avgMs * 1.943844) : null;
    $air = toFloatOrNull($row[7] ?? null);
    $barometer = toFloatOrNull($row[8] ?? null);
    $battery = toIntOrNull($row[9] ?? null);
    $imageAge = toIntOrNull($row[10] ?? null);
    $altitude = toIntOrNull($row[11] ?? null);
    $latitude = toFloatOrNull($row[12] ?? null);
    $longitude = toFloatOrNull($row[13] ?? null);
    $version = trim((string) ($row[14] ?? ''));

    $now = time();
    $uploadAge = $uploadTs ? max(0, $now - $uploadTs) : null;
    $weatherAge = $weatherTs ? max(0, $now - $weatherTs) : null;

    $cameraFile = rtrim($captureDir, '/') . '/' . $stationCode . '1.jpg';
    $cameraFileExists = is_file($cameraFile);
    $cameraFileAge = $cameraFileExists ? max(0, $now - (int) filemtime($cameraFile)) : null;

    $stationState = 'ok';
    $weatherState = 'ok';
    $cameraState = 'ok';
    $batteryState = 'na';
    $summaryState = 'ok';
    $summaryLabel = 'OK';
    $issues = [];

    if ($uploadAge === null || $uploadAge > 900) {
        $stationState = 'offline';
        $summaryState = 'error';
        $summaryLabel = 'Hors ligne';
        $issues[] = 'Aucune remontée récente';
    } elseif ($uploadAge > 300) {
        $stationState = 'warning';
        $summaryState = 'warning';
        $summaryLabel = 'En retard';
        $issues[] = 'Upload ancien';
    }

    $weatherDataPresent = $weatherRaw !== '' && $weatherRaw !== '0' && $weatherRaw !== '0.0';

    if (!$weatherDataPresent) {
        $weatherState = $uploadAge !== null && $uploadAge <= 900 ? 'warning' : 'error';
        $issues[] = 'Capteur météo absent';
        if ($summaryState === 'ok') {
            $summaryState = 'warning';
            $summaryLabel = 'Capteur météo absent';
        }
    } elseif ($weatherTs !== null && $uploadTs !== null && $weatherTs < $uploadTs - 900) {
        $weatherState = 'warning';
        $issues[] = 'Horloge météo décalée';
        if ($summaryState === 'ok') {
            $summaryState = 'warning';
            $summaryLabel = 'Données météo anciennes';
        }
    }

    if ($uploadAge !== null && $uploadAge <= 900) {
        if ($imageAge === null || $imageAge > 7200) {
            $cameraState = 'error';
            $issues[] = 'Caméra absente';
            if ($summaryState !== 'error') {
                $summaryState = 'warning';
                $summaryLabel = 'Caméra KO';
            }
        } elseif ($imageAge > 4500) {
            $cameraState = 'warning';
            $issues[] = 'Caméra en retard';
            if ($summaryState === 'ok') {
                $summaryState = 'warning';
                $summaryLabel = 'Caméra en retard';
            }
        }
    } else {
        $cameraState = 'error';
    }

    if ($battery === null || $battery <= 0) {
        $batteryState = 'na';
    } elseif ($battery < 50) {
        $batteryState = 'error';
        $issues[] = 'Batterie faible';
        if ($summaryState !== 'error') {
            $summaryState = 'warning';
            $summaryLabel = 'Batterie faible';
        }
    } elseif ($battery < 70) {
        $batteryState = 'warning';
        if ($summaryState === 'ok') {
            $summaryState = 'warning';
            $summaryLabel = 'Batterie à surveiller';
        }
    } else {
        $batteryState = 'ok';
    }

    if ($stationState === 'offline') {
        $summaryState = 'error';
        $summaryLabel = 'Hors ligne';
    }

    if ($summaryState === 'ok' && $windAvgKt === 0 && $windDir === 0.0) {
        $summaryLabel = 'Calme';
    }

    return [
        'station_code' => $stationCode,
        'station_name' => $stationName,
        'upload_ts' => $uploadTs,
        'upload_age' => $uploadAge,
        'weather_ts' => $weatherTs,
        'weather_raw' => $weatherRaw,
        'weather_age' => $weatherAge,
        'wind_dir' => $windDir,
        'wind_dir_alpha' => windCompass($windDir),
        'wind_gust_kt' => $windGustKt,
        'wind_avg_kt' => $windAvgKt,
        'air' => $air,
        'barometer' => $barometer,
        'battery' => $battery,
        'image_age' => $imageAge,
        'camera_file_age' => $cameraFileAge,
        'camera_file_exists' => $cameraFileExists,
        'altitude' => $altitude,
        'latitude' => $latitude,
        'longitude' => $longitude,
        'version' => $version,
        'station_state' => $stationState,
        'weather_state' => $weatherState,
        'camera_state' => $cameraState,
        'battery_state' => $batteryState,
        'summary_state' => $summaryState,
        'summary_label' => $summaryLabel,
        'issues' => $issues,
        'sort_weight' => stationSeverity([
            'station_state' => $stationState,
            'weather_state' => $weatherState,
            'camera_state' => $cameraState,
            'battery_state' => $batteryState,
            'summary_state' => $summaryState,
        ]),
    ];
}

$stations = [];
$dbError = null;

try {
    $db = new SQLite3($sqliteDb);
    $results = $db->query('SELECT * FROM station_report ORDER BY station');
    while ($row = $results->fetchArray(SQLITE3_NUM)) {
        $stations[] = summarizeStation($row, $captureDir);
    }
    $db->close();
} catch (Throwable $exception) {
    $dbError = $exception->getMessage();
}

usort(
    $stations,
    static function (array $left, array $right): int {
        $leftPrefixRank = stationPrefixRank($left['station_code']);
        $rightPrefixRank = stationPrefixRank($right['station_code']);
        if ($leftPrefixRank !== $rightPrefixRank) {
            return $leftPrefixRank <=> $rightPrefixRank;
        }

        if ($left['station_code'] !== $right['station_code']) {
            return strcmp($left['station_code'], $right['station_code']);
        }

        if ($left['sort_weight'] !== $right['sort_weight']) {
            return $right['sort_weight'] <=> $left['sort_weight'];
        }

        $leftAge = $left['upload_age'] ?? PHP_INT_MAX;
        $rightAge = $right['upload_age'] ?? PHP_INT_MAX;
        return $rightAge <=> $leftAge;
    }
);

$stats = [
    'total' => count($stations),
    'ok' => 0,
    'warning' => 0,
    'error' => 0,
    'battery_low' => 0,
    'camera_late' => 0,
    'missing_info' => 0,
];

foreach ($stations as $station) {
    if ($station['summary_state'] === 'ok') {
        $stats['ok']++;
    } elseif ($station['summary_state'] === 'warning') {
        $stats['warning']++;
    } else {
        $stats['error']++;
    }

    if ($station['battery_state'] === 'error') {
        $stats['battery_low']++;
    }

    if (in_array($station['camera_state'], ['warning', 'error'], true)) {
        $stats['camera_late']++;
    }

    if (
        $station['battery_state'] === 'na' ||
        $station['weather_state'] === 'error' ||
        $station['camera_state'] === 'error'
    ) {
        $stats['missing_info']++;
    }
}
?>
<!DOCTYPE html>
<html lang="fr">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta name="robots" content="noindex,nofollow">
  <meta name="description" content="État temps réel des stations WindSpots">
  <title>WindSpots Station Status</title>
  <style>
    :root {
      color-scheme: light;
      --bg: #f4f7fb;
      --panel: #ffffff;
      --panel-alt: #eef3f9;
      --text: #152033;
      --muted: #5d6b82;
      --line: #d9e2ec;
      --accent: #0d6efd;
      --ok-bg: #e7f8ee;
      --ok-fg: #11643a;
      --warn-bg: #fff1d6;
      --warn-fg: #9b5b00;
      --error-bg: #ffe2e0;
      --error-fg: #a1271f;
      --na-bg: #edf1f5;
      --na-fg: #627085;
      --shadow: 0 16px 40px rgba(21, 32, 51, 0.08);
    }

    * { box-sizing: border-box; }

    body {
      margin: 0;
      font-family: Arial, Helvetica, sans-serif;
      background: linear-gradient(180deg, #edf4fb 0%, var(--bg) 260px);
      color: var(--text);
    }

    .page {
      width: min(1440px, calc(100vw - 24px));
      margin: 0 auto;
      padding: 24px 0 40px;
    }

    .hero,
    .panel {
      background: var(--panel);
      border: 1px solid rgba(21, 32, 51, 0.06);
      border-radius: 18px;
      box-shadow: var(--shadow);
    }

    .hero {
      display: flex;
      justify-content: space-between;
      gap: 24px;
      align-items: flex-start;
      padding: 24px;
      margin-bottom: 18px;
    }

    .hero h1 {
      margin: 0 0 8px;
      font-size: 2rem;
    }

    .hero p,
    .meta,
    .legend,
    .subline {
      color: var(--muted);
    }

    .stamp {
      text-align: right;
      min-width: 220px;
    }

    .stamp strong {
      display: block;
      font-size: 1.5rem;
      color: var(--text);
    }

    .metrics {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(170px, 1fr));
      gap: 12px;
      margin: 0 0 18px;
    }

    .metric {
      padding: 16px 18px;
      border-radius: 16px;
      background: var(--panel);
      border: 1px solid rgba(21, 32, 51, 0.06);
      box-shadow: var(--shadow);
    }

    .metric strong {
      display: block;
      font-size: 1.8rem;
      margin-top: 6px;
    }

    .metric small {
      color: var(--muted);
    }

    .filters {
      display: flex;
      flex-wrap: wrap;
      gap: 10px;
      margin: 0 0 18px;
    }

    .chip {
      display: inline-flex;
      align-items: center;
      gap: 8px;
      padding: 0;
      border-radius: 999px;
      font-size: 0.92rem;
      font-weight: 700;
      background: transparent;
      color: var(--muted);
    }

    .chip input {
      position: absolute;
      opacity: 0;
      pointer-events: none;
    }

    .chip-label {
      display: inline-flex;
      align-items: center;
      gap: 8px;
      padding: 8px 12px;
      border-radius: 999px;
      background: var(--panel-alt);
      cursor: pointer;
      border: 1px solid transparent;
    }

    .chip-label::before {
      content: '';
      width: 10px;
      height: 10px;
      border-radius: 50%;
      background: currentColor;
      opacity: 0.9;
    }

    .chip-ok { color: var(--ok-fg); }
    .chip-warn { color: var(--warn-fg); }
    .chip-error { color: var(--error-fg); }
    .chip-na { color: var(--na-fg); }
    .chip-ok .chip-label { background: var(--ok-bg); }
    .chip-warn .chip-label { background: var(--warn-bg); }
    .chip-error .chip-label { background: var(--error-bg); }
    .chip-na .chip-label { background: var(--na-bg); }
    .chip input:not(:checked) + .chip-label {
      background: var(--panel-alt);
      color: var(--muted);
      border-color: var(--line);
    }

    .panel {
      overflow: hidden;
    }

    .table-wrap {
      overflow-x: auto;
    }

    table {
      width: 100%;
      border-collapse: collapse;
      min-width: 1180px;
    }

    thead th {
      position: sticky;
      top: 0;
      z-index: 1;
      background: #dfe9f5;
      color: #26415f;
      text-align: left;
      padding: 14px 12px;
      font-size: 0.82rem;
      letter-spacing: 0.04em;
      text-transform: uppercase;
      border-bottom: 1px solid var(--line);
    }

    thead th.sortable {
      cursor: pointer;
      user-select: none;
    }

    thead th.sortable::after {
      content: '↕';
      margin-left: 8px;
      font-size: 0.85em;
      color: #60758f;
    }

    thead th.sort-asc::after {
      content: '↑';
      color: #23486f;
    }

    thead th.sort-desc::after {
      content: '↓';
      color: #23486f;
    }

    tbody td {
      padding: 13px 12px;
      border-bottom: 1px solid var(--line);
      vertical-align: middle;
      font-size: 0.95rem;
    }

    tbody tr:hover {
      background: #f8fbff;
    }

    tbody tr.state-warning {
      background: linear-gradient(90deg, rgba(255, 241, 214, 0.58), transparent 60%);
    }

    tbody tr.state-error {
      background: linear-gradient(90deg, rgba(255, 226, 224, 0.68), transparent 60%);
    }

    .station-name {
      font-weight: 700;
      margin-bottom: 4px;
    }

    .station-label,
    .station-meta {
      font-size: 0.85rem;
      color: var(--muted);
    }

    .station-label {
      margin-bottom: 2px;
    }

    tbody tr.clickable-row {
      cursor: pointer;
    }

    .station-links a {
      color: var(--accent);
      text-decoration: none;
      position: relative;
      z-index: 2;
    }

    .status-badge {
      display: inline-flex;
      align-items: center;
      padding: 7px 10px;
      border-radius: 999px;
      font-size: 0.84rem;
      font-weight: 700;
      white-space: nowrap;
    }

    .status-ok { background: var(--ok-bg); color: var(--ok-fg); }
    .status-warning { background: var(--warn-bg); color: var(--warn-fg); }
    .status-error,
    .status-offline { background: var(--error-bg); color: var(--error-fg); }
    .status-na { background: var(--na-bg); color: var(--na-fg); }

    .gauge {
      display: inline-flex;
      justify-content: center;
      align-items: center;
      min-width: 34px;
      height: 28px;
      border-radius: 10px;
      font-weight: 700;
    }

    .gauge-ok { background: var(--ok-bg); color: var(--ok-fg); }
    .gauge-warning { background: var(--warn-bg); color: var(--warn-fg); }
    .gauge-error,
    .gauge-offline { background: var(--error-bg); color: var(--error-fg); }
    .gauge-na { background: var(--na-bg); color: var(--na-fg); }

    .issues {
      color: var(--muted);
      font-size: 0.86rem;
      line-height: 1.45;
      max-width: 280px;
    }

    .num {
      text-align: center;
      white-space: nowrap;
    }

    .col-compact {
      width: 1%;
      min-width: 52px;
      white-space: nowrap;
      padding-left: 8px;
      padding-right: 8px;
    }

    .col-status {
      width: 1%;
      min-width: 110px;
      white-space: nowrap;
    }

    .col-age {
      width: 1%;
      min-width: 82px;
      white-space: nowrap;
    }

    .footer {
      display: flex;
      justify-content: space-between;
      gap: 12px;
      padding: 16px 4px 0;
      color: var(--muted);
      font-size: 0.9rem;
    }

    .error-box {
      margin-top: 16px;
      padding: 16px 18px;
      border-radius: 14px;
      background: var(--error-bg);
      color: var(--error-fg);
      font-weight: 700;
    }

    @media (max-width: 900px) {
      .page {
        width: min(100vw - 16px, 1440px);
        padding-top: 16px;
      }

      .hero {
        flex-direction: column;
      }

      .stamp {
        text-align: left;
        min-width: 0;
      }
    }
  </style>
</head>
<body onload="setTimeout(function () { location.reload(); }, 60000);">
  <main class="page">
    <section class="hero">
      <div>
        <h1>WindSpots Station Status</h1>
        <p>Vue d'exploitation des stations, capteurs météo et caméras, triée par criticité.</p>
        <div class="legend">S = station/upload, W = station météo, C = caméra. Clique sur un en-tête pour trier. Recharge auto toutes les 60 secondes.</div>
      </div>
      <div class="stamp">
        <div class="meta">Dernier rafraîchissement</div>
        <strong><?= h(date('d/m/Y H:i:s')) ?></strong>
        <div class="subline">Version page <?= h($pageVersion) ?></div>
      </div>
    </section>

    <section class="metrics">
      <article class="metric">
        <small>Stations</small>
        <strong><?= $stats['total'] ?></strong>
      </article>
      <article class="metric">
        <small>OK</small>
        <strong><?= $stats['ok'] ?></strong>
      </article>
      <article class="metric">
        <small>En alerte</small>
        <strong><?= $stats['warning'] ?></strong>
      </article>
      <article class="metric">
        <small>Hors ligne</small>
        <strong><?= $stats['error'] ?></strong>
      </article>
      <article class="metric">
        <small>Batteries faibles</small>
        <strong><?= $stats['battery_low'] ?></strong>
      </article>
      <article class="metric">
        <small>Caméras à surveiller</small>
        <strong><?= $stats['camera_late'] ?></strong>
      </article>
    </section>

    <section class="filters">
      <label class="chip chip-ok">
        <input type="checkbox" data-filter="ok" checked>
        <span class="chip-label">OK (<?= $stats['ok'] ?>)</span>
      </label>
      <label class="chip chip-warn">
        <input type="checkbox" data-filter="warning">
        <span class="chip-label">Retard / anomalie (<?= $stats['warning'] ?>)</span>
      </label>
      <label class="chip chip-error">
        <input type="checkbox" data-filter="error">
        <span class="chip-label">Hors ligne / erreur (<?= $stats['error'] ?>)</span>
      </label>
      <label class="chip chip-na">
        <input type="checkbox" data-filter="missing">
        <span class="chip-label">Information absente (<?= $stats['missing_info'] ?>)</span>
      </label>
    </section>

    <section class="panel">
      <div class="table-wrap">
        <table>
          <thead>
            <tr>
              <th class="sortable sort-asc" data-sort-type="station">Station</th>
              <th class="sortable col-status" data-sort-type="text">Statut</th>
              <th class="sortable col-compact" data-sort-type="text">S</th>
              <th class="sortable col-compact" data-sort-type="text">W</th>
              <th class="sortable col-compact" data-sort-type="text">C</th>
              <th class="sortable col-age" data-sort-type="number">Âge</th>
              <th class="sortable col-compact" data-sort-type="number">Heure</th>
              <th class="sortable col-compact" data-sort-type="number">Date</th>
              <th class="sortable col-compact" data-sort-type="text">Dir</th>
              <th class="sortable col-compact" data-sort-type="number">Vent</th>
              <th class="sortable col-compact" data-sort-type="number">Rafale</th>
              <th class="sortable col-compact" data-sort-type="number">Air</th>
              <th class="sortable col-compact" data-sort-type="number">Pression</th>
              <th class="sortable" data-sort-type="number">Batterie</th>
              <th class="sortable" data-sort-type="number">Caméra</th>
              <th class="sortable" data-sort-type="text">Version</th>
              <th class="sortable" data-sort-type="text">Détails</th>
            </tr>
          </thead>
          <tbody>
            <?php foreach ($stations as $station): ?>
              <?php
              $rowClass = 'state-' . ($station['summary_state'] === 'error' ? 'error' : ($station['summary_state'] === 'warning' ? 'warning' : 'ok'));
              $mapsUrl = null;
              $imageUrl = stationImageUrl($station['station_code']);
              if ($station['latitude'] !== null && $station['longitude'] !== null) {
                  $mapsUrl = 'https://www.google.com/maps/search/?api=1&query=' . rawurlencode((string) $station['latitude'] . ',' . (string) $station['longitude']);
              }
              $cameraText = $station['image_age'] !== null && $station['image_age'] > 0 ? formatAge($station['image_age']) : 'n/a';
              $batteryText = $station['battery'] !== null && $station['battery'] > 0 ? $station['battery'] . '%' : 'n/a';
              $temperatureText = $station['air'] !== null ? number_format($station['air'], 1, '.', '') . ' °C' : 'n/a';
              $barometerText = $station['barometer'] !== null && $station['barometer'] > 0 ? number_format($station['barometer'], 0, '.', '') . ' mb' : 'n/a';
              ?>
              <tr
                class="<?= h($rowClass) ?> clickable-row"
                data-href="<?= h($imageUrl) ?>"
                data-filter-state="<?= h($station['summary_state']) ?>"
                data-filter-missing="<?= h(($station['battery_state'] === 'na' || $station['weather_state'] === 'error' || $station['camera_state'] === 'error') ? '1' : '0') ?>"
              >
                <td data-sort="<?= h(sprintf('%02d|%s|%02d|%010d', stationPrefixRank($station['station_code']), $station['station_code'], 99 - $station['sort_weight'], (int) ($station['upload_age'] ?? PHP_INT_MAX))) ?>">
                  <div class="station-name"><?= h($station['station_code']) ?></div>
                  <?php if ($station['station_name'] !== ''): ?>
                    <div class="station-label"><?= h($station['station_name']) ?></div>
                  <?php endif; ?>
                  <div class="station-meta">
                    <?= $station['altitude'] !== null ? h((string) $station['altitude']) . ' m' : 'Altitude n/a' ?>
                    <?php if ($mapsUrl !== null): ?>
                      <span class="station-links"> · <a href="<?= h($mapsUrl) ?>" target="_blank" rel="noopener noreferrer">Carte</a></span>
                    <?php endif; ?>
                  </div>
                </td>
                <td class="col-status" data-sort="<?= h($station['summary_label']) ?>"><span class="status-badge status-<?= h($station['summary_state'] === 'error' ? 'error' : ($station['summary_state'] === 'warning' ? 'warning' : 'ok')) ?>"><?= h($station['summary_label']) ?></span></td>
                <td class="num col-compact" data-sort="<?= h($station['station_state']) ?>"><span class="<?= h(gaugeClass($station['station_state'])) ?>">S</span></td>
                <td class="num col-compact" data-sort="<?= h($station['weather_state']) ?>"><span class="<?= h(gaugeClass($station['weather_state'])) ?>">W</span></td>
                <td class="num col-compact" data-sort="<?= h($station['camera_state']) ?>"><span class="<?= h(gaugeClass($station['camera_state'])) ?>">C</span></td>
                <td class="col-age" data-sort="<?= h((string) ($station['upload_age'] ?? PHP_INT_MAX)) ?>"><?= h(formatAge($station['upload_age'])) ?></td>
                <td class="num col-compact" data-sort="<?= h((string) ($station['upload_ts'] ?? 0)) ?>"><?= h(formatClock($station['upload_ts'])) ?></td>
                <td class="num col-compact" data-sort="<?= h($station['upload_ts'] ? date('Ymd', $station['upload_ts']) : '0') ?>"><?= h(formatDay($station['upload_ts'])) ?></td>
                <td class="num col-compact" data-sort="<?= h((string) ($station['wind_dir'] ?? -1)) ?>"><?= h($station['wind_dir_alpha']) ?></td>
                <td class="num col-compact" data-sort="<?= h((string) ($station['wind_avg_kt'] ?? -1)) ?>"><?= $station['wind_avg_kt'] !== null ? h((string) $station['wind_avg_kt']) . ' kt' : 'n/a' ?></td>
                <td class="num col-compact" data-sort="<?= h((string) ($station['wind_gust_kt'] ?? -1)) ?>"><?= $station['wind_gust_kt'] !== null ? h((string) $station['wind_gust_kt']) . ' kt' : 'n/a' ?></td>
                <td class="num col-compact" data-sort="<?= h((string) ($station['air'] ?? -9999)) ?>"><?= h($temperatureText) ?></td>
                <td class="num col-compact" data-sort="<?= h((string) ($station['barometer'] ?? -1)) ?>"><?= h($barometerText) ?></td>
                <td class="num" data-sort="<?= h((string) ($station['battery'] ?? -1)) ?>"><span class="status-badge status-<?= h($station['battery_state']) ?>"><?= h($batteryText) ?></span></td>
                <td class="num" data-sort="<?= h((string) ($station['image_age'] ?? PHP_INT_MAX)) ?>"><?= h($cameraText) ?></td>
                <td class="num" data-sort="<?= h($station['version'] !== '' ? $station['version'] : '0') ?>"><?= h($station['version'] !== '' ? $station['version'] : 'n/a') ?></td>
                <td class="issues" data-sort="<?= h(implode(' | ', $station['issues'])) ?>">
                  <?php if ($station['issues'] !== []): ?>
                    <?= h(implode(' · ', $station['issues'])) ?>
                  <?php endif; ?>
                </td>
              </tr>
            <?php endforeach; ?>
          </tbody>
        </table>
      </div>
    </section>

    <?php if ($dbError !== null): ?>
      <div class="error-box">Erreur d'ouverture SQLite: <?= h($dbError) ?></div>
    <?php endif; ?>

    <footer class="footer">
      <div>Copyright © WindSpots 2026</div>
      <div>Source: <?= h($sqliteDb) ?></div>
    </footer>
  </main>
  <script>
    (() => {
      const table = document.querySelector('table');
      if (!table) return;

      const storageKey = 'windspots-station-report-ui';
      const headers = Array.from(table.querySelectorAll('thead th.sortable'));
      const tbody = table.querySelector('tbody');
      const filterInputs = Array.from(document.querySelectorAll('.filters input[type="checkbox"]'));
      const defaultSortIndex = headers.findIndex((header) => header.dataset.sortType === 'station');
      const prefixRank = (value) => {
        if (value.startsWith('CHGE')) return 0;
        if (value.startsWith('CHVD')) return 1;
        if (value.startsWith('CHVS')) return 2;
        return 99;
      };

      const loadState = () => {
        try {
          return JSON.parse(window.localStorage.getItem(storageKey) || '{}');
        } catch (_error) {
          return {};
        }
      };

      const saveState = (state) => {
        try {
          window.localStorage.setItem(storageKey, JSON.stringify(state));
        } catch (_error) {
          // Ignore storage failures and keep UI working.
        }
      };

      const state = {
        filters: {
          ok: true,
          warning: false,
          error: false,
          missing: false
        },
        sortIndex: defaultSortIndex >= 0 ? defaultSortIndex : 0,
        sortDirection: 'asc',
        ...loadState()
      };

      if (!state.filters || typeof state.filters !== 'object') {
        state.filters = { ok: true, warning: false, error: false, missing: false };
      }

      const compareValues = (left, right, type, direction) => {
        let result = 0;
        if (type === 'number') {
          result = Number(left) - Number(right);
        } else if (type === 'station') {
          const leftValue = String(left);
          const rightValue = String(right);
          result = prefixRank(leftValue) - prefixRank(rightValue);
          if (result === 0) {
            result = leftValue.localeCompare(rightValue, 'fr', { numeric: true, sensitivity: 'base' });
          }
        } else {
          result = String(left).localeCompare(String(right), 'fr', { numeric: true, sensitivity: 'base' });
        }

        return direction === 'asc' ? result : -result;
      };

      const sortRows = (index, direction) => {
        const header = headers[index];
        if (!header) return;

        headers.forEach((item) => {
          item.dataset.direction = '';
          item.classList.remove('sort-asc', 'sort-desc');
        });
        header.dataset.direction = direction;
        header.classList.add(direction === 'asc' ? 'sort-asc' : 'sort-desc');

        const rows = Array.from(tbody.querySelectorAll('tr'));
        rows.sort((leftRow, rightRow) => {
          const leftCell = leftRow.children[index];
          const rightCell = rightRow.children[index];
          const leftValue = leftCell.dataset.sort ?? leftCell.textContent.trim();
          const rightValue = rightCell.dataset.sort ?? rightCell.textContent.trim();
          return compareValues(leftValue, rightValue, header.dataset.sortType || 'text', direction);
        });

        rows.forEach((row) => tbody.appendChild(row));
        state.sortIndex = index;
        state.sortDirection = direction;
        saveState(state);
      };

      const applyFilters = () => {
        const enabled = new Set(
          filterInputs
            .filter((input) => input.checked)
            .map((input) => input.dataset.filter)
        );

        const rows = Array.from(tbody.querySelectorAll('tr'));
        rows.forEach((row) => {
          const state = row.dataset.filterState;
          const missing = row.dataset.filterMissing === '1';
          const show =
            (state === 'ok' && enabled.has('ok')) ||
            (state === 'warning' && enabled.has('warning')) ||
            (state === 'error' && enabled.has('error')) ||
            (missing && enabled.has('missing'));

          row.style.display = show ? '' : 'none';
        });

        state.filters = filterInputs.reduce((accumulator, input) => {
          accumulator[input.dataset.filter] = input.checked;
          return accumulator;
        }, {});
        saveState(state);
      };

      filterInputs.forEach((input) => {
        const savedValue = state.filters[input.dataset.filter];
        input.checked = typeof savedValue === 'boolean' ? savedValue : input.checked;
      });

      headers.forEach((header, index) => {
        header.addEventListener('click', () => {
          const isCurrentSort = state.sortIndex === index;
          const current = isCurrentSort && state.sortDirection === 'asc' ? 'desc' : 'asc';
          sortRows(index, current);
        });
      });

      filterInputs.forEach((input) => {
        input.addEventListener('change', applyFilters);
      });

      tbody.addEventListener('click', (event) => {
        const interactive = event.target.closest('a, button');
        if (interactive) return;

        const row = event.target.closest('tr[data-href]');
        if (!row) return;
        window.location.href = row.dataset.href;
      });

      sortRows(state.sortIndex, state.sortDirection === 'desc' ? 'desc' : 'asc');
      applyFilters();
    })();
  </script>
</body>
</html>
