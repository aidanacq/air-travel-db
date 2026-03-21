const API = '';
let oneHopMap = null;
let mapLayers = [];

// ── Utilities ────────────────────────────────────────────────────────────────

async function api(method, path, body = null) {
    const opts = { method, headers: { 'Content-Type': 'application/json' } };
    if (body) opts.body = JSON.stringify(body);
    const res = await fetch(API + path, opts);
    return { status: res.status, data: await res.json() };
}

function showToast(msg, type = 'info') {
    const c = document.getElementById('toast-container');
    const t = document.createElement('div');
    t.className = `toast ${type}`;
    t.textContent = msg;
    c.appendChild(t);
    setTimeout(() => t.remove(), 3000);
}

function showLoading(el) {
    el.innerHTML = '<div class="loading-text"><div class="spinner"></div> Loading...</div>';
}

function showError(el, msg) {
    el.innerHTML = `<div class="error-msg">${msg}</div>`;
}

function entityCard(data, title) {
    let html = '<div class="result-card">';
    if (title) html += `<h4 style="color:var(--accent);margin-bottom:.5rem">${title}</h4>`;
    html += '<dl>';
    for (const [k, v] of Object.entries(data)) {
        html += `<dt>${k}</dt><dd>${v === '' ? '—' : v}</dd>`;
    }
    html += '</dl></div>';
    return html;
}

function buildTable(headers, rows, numCols = []) {
    let html = '<div class="data-table-wrapper"><table class="data-table"><thead><tr>';
    headers.forEach((h, i) => {
        html += `<th class="${numCols.includes(i) ? 'num' : ''}">${h}</th>`;
    });
    html += '</tr></thead><tbody>';
    rows.forEach(row => {
        html += '<tr>';
        row.forEach((cell, i) => {
            html += `<td class="${numCols.includes(i) ? 'num' : ''}">${cell === '' || cell === null || cell === undefined ? '—' : cell}</td>`;
        });
        html += '</tr>';
    });
    html += '</tbody></table></div>';
    return html;
}

function paginate(items, page, perPage) {
    const total = Math.ceil(items.length / perPage);
    const start = (page - 1) * perPage;
    return { items: items.slice(start, start + perPage), page, total, count: items.length };
}

function paginationHtml(pg, onPage) {
    return `<div class="pagination">
        <button ${pg.page <= 1 ? 'disabled' : ''} onclick="${onPage}(1)">&#171; First</button>
        <button ${pg.page <= 1 ? 'disabled' : ''} onclick="${onPage}(${pg.page - 1})">&#8249; Prev</button>
        <span class="page-info">Page
            <input type="number" class="page-input" value="${pg.page}" min="1" max="${pg.total}"
                onchange="${onPage}(parseInt(this.value)||1)"
                onkeydown="if(event.key==='Enter'){${onPage}(parseInt(this.value)||1);event.preventDefault();}">
            of ${pg.total}</span>
        <span class="page-count">(${pg.count} records)</span>
        <button ${pg.page >= pg.total ? 'disabled' : ''} onclick="${onPage}(${pg.page + 1})">Next &#8250;</button>
        <button ${pg.page >= pg.total ? 'disabled' : ''} onclick="${onPage}(${pg.total})">Last &#187;</button>
    </div>`;
}

// ── Tab Navigation ───────────────────────────────────────────────────────────

document.querySelectorAll('.nav-link').forEach(link => {
    link.addEventListener('click', e => {
        e.preventDefault();
        const tab = link.dataset.tab;
        document.querySelectorAll('.nav-link').forEach(l => l.classList.remove('active'));
        link.classList.add('active');
        document.querySelectorAll('.tab-content').forEach(s => s.classList.remove('active'));
        document.getElementById('tab-' + tab).classList.add('active');
        if (tab === 'onehop' && oneHopMap) {
            setTimeout(() => oneHopMap.invalidateSize(), 100);
        }
        if (tab === 'routes' && routesMap) {
            setTimeout(() => routesMap.invalidateSize(), 100);
        }
    });
});

document.querySelector('.nav-toggle').addEventListener('click', () => {
    document.querySelector('.nav-links').classList.toggle('open');
});

document.querySelectorAll('.report-tab').forEach(btn => {
    btn.addEventListener('click', () => {
        document.querySelectorAll('.report-tab').forEach(b => b.classList.remove('active'));
        btn.classList.add('active');
        document.querySelectorAll('.report-panel').forEach(p => p.classList.remove('active'));
        document.getElementById('report-' + btn.dataset.report).classList.add('active');
    });
});

document.querySelectorAll('.manage-tab').forEach(btn => {
    btn.addEventListener('click', () => {
        document.querySelectorAll('.manage-tab').forEach(b => b.classList.remove('active'));
        btn.classList.add('active');
        document.querySelectorAll('.manage-panel').forEach(p => p.classList.remove('active'));
        document.getElementById('manage-' + btn.dataset.entity).classList.add('active');
    });
});

// ── Search ───────────────────────────────────────────────────────────────────

async function searchAirline() {
    const iata = document.getElementById('airline-iata').value.trim().toUpperCase();
    const el = document.getElementById('airline-result');
    if (!iata) { showError(el, 'Enter an IATA code'); return; }
    showLoading(el);
    const { status, data } = await api('GET', `/api/airline/${iata}`);
    if (data.error) { showError(el, data.error); return; }
    el.innerHTML = entityCard(data, `${data.name} (${data.iata})`);
}

async function searchAirport() {
    const iata = document.getElementById('airport-iata').value.trim().toUpperCase();
    const el = document.getElementById('airport-result');
    if (!iata) { showError(el, 'Enter an IATA code'); return; }
    showLoading(el);
    const { status, data } = await api('GET', `/api/airport/${iata}`);
    if (data.error) { showError(el, data.error); return; }
    el.innerHTML = entityCard(data, `${data.name} (${data.iata})`);
}

// ── Reports ──────────────────────────────────────────────────────────────────

let airlineRoutesCache = null;
let airlineRoutesTitle = '';
async function airlineRoutesReport() {
    const iata = document.getElementById('report-airline-iata').value.trim().toUpperCase();
    const el = document.getElementById('report-airline-routes-result');
    if (!iata) { showError(el, 'Enter an airline IATA code'); return; }
    showLoading(el);
    const { data } = await api('GET', `/api/report/airline-routes/${iata}`);
    if (data.error) { showError(el, data.error); return; }
    airlineRoutesCache = data.airports;
    airlineRoutesTitle = `${data.airline.name} — ${data.totalAirports} airports`;
    renderAirlineRoutes(1);
}
function renderAirlineRoutes(page) {
    page = Math.max(1, Math.min(Math.ceil(airlineRoutesCache.length / 50), page));
    const el = document.getElementById('report-airline-routes-result');
    const pg = paginate(airlineRoutesCache, page, 50);
    const rows = pg.items.map(a => [a.iata, a.name, a.city, a.country, a.routeCount]);
    el.innerHTML = `<h4 style="color:var(--accent);margin-bottom:.5rem">${airlineRoutesTitle}</h4>` +
        buildTable(['IATA', 'Airport', 'City', 'Country', '# Routes'], rows, [4]) +
        paginationHtml(pg, 'renderAirlineRoutes');
}

let airportRoutesCache = null;
let airportRoutesTitle = '';
async function airportRoutesReport() {
    const iata = document.getElementById('report-airport-iata').value.trim().toUpperCase();
    const el = document.getElementById('report-airport-routes-result');
    if (!iata) { showError(el, 'Enter an airport IATA code'); return; }
    showLoading(el);
    const { data } = await api('GET', `/api/report/airport-routes/${iata}`);
    if (data.error) { showError(el, data.error); return; }
    airportRoutesCache = data.airlines;
    airportRoutesTitle = `${data.airport.name} — ${data.totalAirlines} airlines`;
    renderAirportRoutes(1);
}
function renderAirportRoutes(page) {
    page = Math.max(1, Math.min(Math.ceil(airportRoutesCache.length / 50), page));
    const el = document.getElementById('report-airport-routes-result');
    const pg = paginate(airportRoutesCache, page, 50);
    const rows = pg.items.map(a => [a.iata, a.name, a.country, a.routeCount]);
    el.innerHTML = `<h4 style="color:var(--accent);margin-bottom:.5rem">${airportRoutesTitle}</h4>` +
        buildTable(['IATA', 'Airline', 'Country', '# Routes'], rows, [3]) +
        paginationHtml(pg, 'renderAirportRoutes');
}

let allAirlinesData = null;
async function allAirlinesReport(page = 1) {
    const el = document.getElementById('report-all-airlines-result');
    if (!allAirlinesData) {
        showLoading(el);
        const { data } = await api('GET', '/api/report/airlines');
        allAirlinesData = data.airlines;
    }
    page = Math.max(1, Math.min(Math.ceil(allAirlinesData.length / 50), page));
    const pg = paginate(allAirlinesData, page, 50);
    const rows = pg.items.map(a => [a.iata, a.name, a.icao, a.country, a.active ? 'Yes' : 'No']);
    el.innerHTML = `<div class="table-info">Total: ${pg.count} airlines</div>` +
        buildTable(['IATA', 'Name', 'ICAO', 'Country', 'Active'], rows) +
        paginationHtml(pg, 'allAirlinesReport');
}

let allAirportsData = null;
async function allAirportsReport(page = 1) {
    const el = document.getElementById('report-all-airports-result');
    if (!allAirportsData) {
        showLoading(el);
        const { data } = await api('GET', '/api/report/airports');
        allAirportsData = data.airports;
    }
    page = Math.max(1, Math.min(Math.ceil(allAirportsData.length / 50), page));
    const pg = paginate(allAirportsData, page, 50);
    const rows = pg.items.map(a => [a.iata, a.name, a.city, a.country, a.latitude.toFixed(4), a.longitude.toFixed(4)]);
    el.innerHTML = `<div class="table-info">Total: ${pg.count} airports</div>` +
        buildTable(['IATA', 'Name', 'City', 'Country', 'Lat', 'Lon'], rows, [4, 5]) +
        paginationHtml(pg, 'allAirportsReport');
}

// ── CRUD ─────────────────────────────────────────────────────────────────────

function formData(form) {
    const fd = new FormData(form);
    const obj = {};
    for (const [k, v] of fd.entries()) {
        if (v !== '') obj[k] = v;
    }
    return obj;
}

async function addAirline(e) {
    e.preventDefault();
    const d = formData(e.target);
    d.active = e.target.querySelector('[name=active]').checked;
    const { data } = await api('POST', '/api/airline', d);
    if (data.error) { showToast(data.error, 'error'); return; }
    showToast(`Airline ${d.iata} added`, 'success');
    e.target.reset();
    allAirlinesData = null;
}

async function editAirline(e) {
    e.preventDefault();
    const d = formData(e.target);
    const iata = d.iata; delete d.iata;
    const { data } = await api('PUT', `/api/airline/${iata}`, d);
    if (data.error) { showToast(data.error, 'error'); return; }
    showToast(`Airline ${iata} updated`, 'success');
    allAirlinesData = null;
}

async function deleteAirline(e) {
    e.preventDefault();
    const iata = e.target.querySelector('[name=iata]').value.trim().toUpperCase();
    if (!confirm(`Remove airline ${iata} and all its routes?`)) return;
    const { data } = await api('DELETE', `/api/airline/${iata}`);
    if (data.error) { showToast(data.error, 'error'); return; }
    showToast(`Airline ${iata} removed`, 'success');
    e.target.reset();
    allAirlinesData = null;
}

async function addAirport(e) {
    e.preventDefault();
    const d = formData(e.target);
    if (d.latitude) d.latitude = parseFloat(d.latitude);
    if (d.longitude) d.longitude = parseFloat(d.longitude);
    const { data } = await api('POST', '/api/airport', d);
    if (data.error) { showToast(data.error, 'error'); return; }
    showToast(`Airport ${d.iata} added`, 'success');
    e.target.reset();
    allAirportsData = null;
}

async function editAirport(e) {
    e.preventDefault();
    const d = formData(e.target);
    const iata = d.iata; delete d.iata;
    const { data } = await api('PUT', `/api/airport/${iata}`, d);
    if (data.error) { showToast(data.error, 'error'); return; }
    showToast(`Airport ${iata} updated`, 'success');
    allAirportsData = null;
}

async function deleteAirport(e) {
    e.preventDefault();
    const iata = e.target.querySelector('[name=iata]').value.trim().toUpperCase();
    if (!confirm(`Remove airport ${iata} and all its routes?`)) return;
    const { data } = await api('DELETE', `/api/airport/${iata}`);
    if (data.error) { showToast(data.error, 'error'); return; }
    showToast(`Airport ${iata} removed`, 'success');
    e.target.reset();
    allAirportsData = null;
}

async function addRoute(e) {
    e.preventDefault();
    const d = formData(e.target);
    d.airlineId = parseInt(d.airlineId);
    d.sourceAirportId = parseInt(d.sourceAirportId);
    d.destAirportId = parseInt(d.destAirportId);
    d.stops = parseInt(d.stops || '0');
    const { data } = await api('POST', '/api/route', d);
    if (data.error) { showToast(data.error, 'error'); return; }
    showToast('Route added', 'success');
    e.target.reset();
}

async function editRoute(e) {
    e.preventDefault();
    const d = formData(e.target);
    const body = {
        sourceAirportId: parseInt(d.sourceAirportId),
        destAirportId: parseInt(d.destAirportId),
        airlineId: parseInt(d.airlineId),
        updates: {}
    };
    if (d.newAirlineId) body.updates.airlineId = parseInt(d.newAirlineId);
    if (d.newSourceAirportId) body.updates.sourceAirportId = parseInt(d.newSourceAirportId);
    if (d.newDestAirportId) body.updates.destAirportId = parseInt(d.newDestAirportId);
    if (d.stops) body.updates.stops = parseInt(d.stops);
    if (d.equipment) body.updates.equipment = d.equipment;
    const { data } = await api('PUT', '/api/route', body);
    if (data.error) { showToast(data.error, 'error'); return; }
    showToast('Route updated', 'success');
    e.target.reset();
}

async function deleteRoute(e) {
    e.preventDefault();
    const d = formData(e.target);
    if (!confirm('Remove this route?')) return;
    const { data } = await api('DELETE', `/api/route?srcId=${d.srcId}&dstId=${d.dstId}&airlineId=${d.airlineId}`);
    if (data.error) { showToast(data.error, 'error'); return; }
    showToast('Route removed', 'success');
    e.target.reset();
}

// ── One-Hop ──────────────────────────────────────────────────────────────────

function initMap() {
    if (oneHopMap) return;
    const container = document.getElementById('onehop-map');
    container.classList.add('visible');
    oneHopMap = L.map(container).setView([30, 0], 2);
    L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
        attribution: '&copy; OpenStreetMap contributors',
        maxZoom: 18
    }).addTo(oneHopMap);
}

function clearMapLayers() {
    mapLayers.forEach(l => oneHopMap.removeLayer(l));
    mapLayers = [];
}

const COLORS = ['#58a6ff', '#3fb950', '#e3b341', '#f85149', '#bc8cff'];

let oneHopPairsCache = null;
let oneHopSummaryHtml = '';

async function searchOneHop() {
    const src = document.getElementById('onehop-src').value.trim().toUpperCase();
    const dst = document.getElementById('onehop-dst').value.trim().toUpperCase();
    const el = document.getElementById('onehop-result');
    if (!src || !dst) { showError(el, 'Enter both source and destination IATA codes'); return; }
    showLoading(el);

    const { data } = await api('GET', `/api/onehop?src=${src}&dst=${dst}`);
    if (data.error) { showError(el, data.error); return; }

    initMap();
    clearMapLayers();

    const srcPos = [data.source.latitude, data.source.longitude];
    const dstPos = [data.destination.latitude, data.destination.longitude];

    const srcMarker = L.marker(srcPos).addTo(oneHopMap).bindPopup(`<b>${data.source.iata}</b><br>${data.source.name}`);
    const dstMarker = L.marker(dstPos).addTo(oneHopMap).bindPopup(`<b>${data.destination.iata}</b><br>${data.destination.name}`);
    mapLayers.push(srcMarker, dstMarker);

    if (data.topIntermediates && data.topIntermediates.length > 0) {
        data.topIntermediates.forEach((item, i) => {
            const ap = item.airport;
            const pos = [ap.latitude, ap.longitude];
            const color = COLORS[i % COLORS.length];
            const circle = L.circleMarker(pos, { radius: 8, color, fillColor: color, fillOpacity: .7 })
                .addTo(oneHopMap)
                .bindPopup(`<b>#${i + 1}: ${ap.iata}</b><br>${ap.name}<br>${item.routePairCount} route pairs`);
            const line1 = L.polyline([srcPos, pos], { color, weight: 2, opacity: .6, dashArray: '6 4' }).addTo(oneHopMap);
            const line2 = L.polyline([pos, dstPos], { color, weight: 2, opacity: .6, dashArray: '6 4' }).addTo(oneHopMap);
            mapLayers.push(circle, line1, line2);
        });

        const allPoints = [srcPos, dstPos, ...data.topIntermediates.map(t => [t.airport.latitude, t.airport.longitude])];
        oneHopMap.fitBounds(allPoints, { padding: [40, 40] });
    } else {
        oneHopMap.fitBounds([srcPos, dstPos], { padding: [40, 40] });
    }

    if (data.routePairs.length === 0) {
        el.innerHTML = '<div class="error-msg">No one-hop route pairs found between these airports.</div>';
        return;
    }

    let summary = `<h4 style="color:var(--accent);margin:.75rem 0">${data.source.iata} &#10230; X &#10230; ${data.destination.iata} — ${data.totalPairs} route pairs found</h4>`;
    if (data.topIntermediates.length > 0) {
        summary += '<div class="top-intermediates"><strong>Top intermediate airports by # route pairs:</strong><ol>';
        data.topIntermediates.forEach((t, i) => {
            const color = COLORS[i % COLORS.length];
            summary += `<li><span class="color-dot" style="background:${color}"></span><strong>${t.airport.iata}</strong> ${t.airport.name} — ${t.routePairCount} pairs</li>`;
        });
        summary += '</ol></div>';
    }
    oneHopSummaryHtml = summary;
    oneHopPairsCache = data.routePairs;
    renderOneHopPairs(1);
}

function renderOneHopPairs(page) {
    page = Math.max(1, Math.min(Math.ceil(oneHopPairsCache.length / 50), page));
    const el = document.getElementById('onehop-result');
    const pg = paginate(oneHopPairsCache, page, 50);
    const rows = pg.items.map(p => [
        p.intermediateIata, p.intermediateName,
        p.airline1, p.airline2,
        p.leg1Miles.toLocaleString(), p.leg2Miles.toLocaleString(), p.totalMiles.toLocaleString()
    ]);
    el.innerHTML = oneHopSummaryHtml +
        buildTable(['Via IATA', 'Via Airport', 'Airline Leg 1', 'Airline Leg 2', 'Leg 1 (mi)', 'Leg 2 (mi)', 'Total (mi)'], rows, [4, 5, 6]) +
        paginationHtml(pg, 'renderOneHopPairs');
}

// ── Routes ───────────────────────────────────────────────────────────────────

let routesMap = null;
let routesMapLayers = [];
let routesSummaryHtml = '';
let routesDataCache = null;

const ROUTE_COLORS = ['#58a6ff', '#3fb950', '#e3b341', '#f85149', '#bc8cff',
                      '#f0883e', '#a5d6ff', '#7ee787', '#d2a8ff', '#ff7b72'];

function toggleAnyAirline() {
    const checked = document.getElementById('routes-any-airline').checked;
    const row = document.getElementById('routes-airline-row');
    row.style.display = checked ? 'none' : '';
    if (checked) {
        document.getElementById('routes-airline').value = '';
        document.getElementById('routes-backup1').value = '';
        document.getElementById('routes-backup2').value = '';
    }
}

function initRoutesMap() {
    if (routesMap) return;
    const container = document.getElementById('routes-map');
    container.classList.add('visible');
    routesMap = L.map(container).setView([30, 0], 2);
    L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
        attribution: '&copy; OpenStreetMap contributors',
        maxZoom: 18
    }).addTo(routesMap);
}

function clearRoutesMapLayers() {
    routesMapLayers.forEach(l => routesMap.removeLayer(l));
    routesMapLayers = [];
}

async function searchRoutes() {
    const src = document.getElementById('routes-src').value.trim().toUpperCase();
    const dst = document.getElementById('routes-dst').value.trim().toUpperCase();
    const anyAirline = document.getElementById('routes-any-airline').checked;
    const airline = anyAirline ? '' : document.getElementById('routes-airline').value.trim().toUpperCase();
    const backup1 = anyAirline ? '' : document.getElementById('routes-backup1').value.trim().toUpperCase();
    const backup2 = anyAirline ? '' : document.getElementById('routes-backup2').value.trim().toUpperCase();
    const stops = document.getElementById('routes-stops').value || '0';
    const stopMode = document.getElementById('routes-stop-mode').value;
    const routeCount = document.getElementById('routes-count').value || '5';
    const el = document.getElementById('routes-result');

    if (!src || !dst) { showError(el, 'Enter both source and destination airport IATA codes.'); return; }
    if (!anyAirline && !airline) { showError(el, 'Enter a primary airline IATA code, or check "Any Airline".'); return; }

    const stopsNum = parseInt(stops);
    if (isNaN(stopsNum) || stopsNum < 0 || stopsNum > 5) {
        showError(el, 'Number of stops must be between 0 and 5.');
        return;
    }

    const countNum = parseInt(routeCount);
    if (isNaN(countNum) || countNum < 1 || countNum > 10) {
        showError(el, 'Number of routes must be between 1 and 10.');
        return;
    }

    if (src === dst) { showError(el, 'Source and destination airports must be different.'); return; }

    if (!anyAirline) {
        if (backup2 && !backup1) {
            showError(el, 'Please specify Backup Airline 1 before Backup Airline 2.');
            return;
        }
        if ((backup1 && backup1 === airline) || (backup2 && backup2 === airline) ||
            (backup1 && backup2 && backup1 === backup2)) {
            showError(el, 'Airline codes must be unique — no duplicates between primary and backup airlines.');
            return;
        }
    }

    showLoading(el);

    const exact = stopMode === 'exact';
    let url = `/api/routes?src=${src}&dst=${dst}&stops=${stopsNum}&count=${countNum}`;
    if (airline) url += `&airline=${airline}`;
    if (backup1) url += `&backup1=${backup1}`;
    if (backup2) url += `&backup2=${backup2}`;
    if (exact) url += `&exact=true`;

    const { data } = await api('GET', url);
    if (data.error) { showError(el, data.error); return; }

    initRoutesMap();
    clearRoutesMapLayers();

    const srcPos = [data.source.latitude, data.source.longitude];
    const dstPos = [data.destination.latitude, data.destination.longitude];

    const srcMarker = L.marker(srcPos).addTo(routesMap)
        .bindPopup(`<b>${data.source.iata}</b><br>${data.source.name}`);
    const dstMarker = L.marker(dstPos).addTo(routesMap)
        .bindPopup(`<b>${data.destination.iata}</b><br>${data.destination.name}`);
    routesMapLayers.push(srcMarker, dstMarker);

    const allPoints = [srcPos, dstPos];
    const seenIntermediates = new Set();

    data.routes.forEach((route, i) => {
        const color = ROUTE_COLORS[i % ROUTE_COLORS.length];
        route.legs.forEach(leg => {
            const from = [leg.fromLat, leg.fromLon];
            const to = [leg.toLat, leg.toLon];
            const line = L.polyline([from, to], {
                color, weight: 2.5, opacity: 0.7, dashArray: '6 4'
            }).addTo(routesMap);
            routesMapLayers.push(line);
        });

        for (let a = 1; a < route.airports.length - 1; a++) {
            const ap = route.airports[a];
            const key = ap.iata;
            if (seenIntermediates.has(key)) continue;
            seenIntermediates.add(key);
            const pos = [ap.latitude, ap.longitude];
            allPoints.push(pos);
            const circle = L.circleMarker(pos, {
                radius: 7, color, fillColor: color, fillOpacity: 0.7
            }).addTo(routesMap).bindPopup(`<b>${ap.iata}</b><br>${ap.name}<br>${ap.city}, ${ap.country}`);
            routesMapLayers.push(circle);
        }
    });

    routesMap.fitBounds(allPoints, { padding: [40, 40] });

    const modeLabel = data.exactStops ? 'exactly' : 'up to';
    const airlineLabel = data.anyAirline ? 'any airline' :
        data.airlines.map(a => `${a.name} (${a.iata})`).join(', ');
    let summary = `<h4 style="color:var(--accent);margin:.75rem 0">${data.source.iata} &#10230; ${data.destination.iata} — ${data.totalRoutes} optimal route(s) found</h4>`;
    summary += `<div class="routes-meta">Direct distance: <strong>${data.directDistance.toLocaleString()} mi</strong> · ${modeLabel} ${data.maxStopsUsed} stop(s) · ${airlineLabel}</div>`;

    summary += '<div class="top-intermediates"><strong>Routes (by total distance):</strong><ol>';
    data.routes.forEach((route, i) => {
        const color = ROUTE_COLORS[i % ROUTE_COLORS.length];
        const pathStr = route.airports.map(a => a.iata).join(' → ');
        const airlineStr = route.legs.map(l => l.airlineIata).join(', ');
        summary += `<li><span class="color-dot" style="background:${color}"></span>` +
            `<strong>${pathStr}</strong> — ${route.totalDistance.toLocaleString()} mi ` +
            `(${route.stops} stop${route.stops !== 1 ? 's' : ''}, via ${airlineStr})</li>`;
    });
    summary += '</ol></div>';

    routesSummaryHtml = summary;
    routesDataCache = data.routes;
    renderRouteDetails();
}

function renderRouteDetails() {
    const el = document.getElementById('routes-result');
    let html = routesSummaryHtml;

    html += '<div class="routes-detail-list">';
    routesDataCache.forEach((route, i) => {
        const color = ROUTE_COLORS[i % ROUTE_COLORS.length];
        html += `<div class="route-detail-card" style="border-left: 3px solid ${color}">`;
        html += `<h4 style="color:${color}">Route #${route.rank}: ${route.airports.map(a => a.iata).join(' → ')}</h4>`;
        html += `<div class="route-stats">` +
            `<span>Total: <strong>${route.totalDistance.toLocaleString()} mi</strong></span>` +
            `<span>Stops: <strong>${route.stops}</strong></span></div>`;

        const headers = ['Leg', 'From', 'To', 'Airline', 'Distance'];
        const rows = route.legs.map((leg, l) => [
            `${l + 1}`,
            `${leg.fromIata} — ${leg.fromName}`,
            `${leg.toIata} — ${leg.toName}`,
            `${leg.airlineName} (${leg.airlineIata})`,
            `${leg.distance.toLocaleString()} mi`
        ]);
        html += buildTable(headers, rows, [0, 4]);
        html += '</div>';
    });
    html += '</div>';

    el.innerHTML = html;
}

// ── Code ─────────────────────────────────────────────────────────────────────

async function loadCode() {
    const el = document.getElementById('code-result');
    showLoading(el);
    const { data } = await api('GET', '/api/code');
    if (!data.code) { showError(el, 'Could not load source code'); return; }
    const escaped = data.code.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;');
    el.innerHTML = `<pre><code class="language-cpp">${escaped}</code></pre>`;
    hljs.highlightAll();
}

// ── Stats & Init ─────────────────────────────────────────────────────────────

function animateCounter(el, target) {
    const duration = 1200;
    const start = performance.now();
    function step(now) {
        const progress = Math.min((now - start) / duration, 1);
        const ease = 1 - Math.pow(1 - progress, 3);
        el.textContent = Math.floor(target * ease).toLocaleString();
        if (progress < 1) requestAnimationFrame(step);
    }
    requestAnimationFrame(step);
}

async function loadStats() {
    try {
        const { data } = await api('GET', '/api/stats');
        animateCounter(document.getElementById('stat-airlines'), data.airlines);
        animateCounter(document.getElementById('stat-airports'), data.airports);
        animateCounter(document.getElementById('stat-routes'), data.routes);
        document.getElementById('about-airlines').textContent = data.airlines.toLocaleString();
        document.getElementById('about-airports').textContent = data.airports.toLocaleString();
        document.getElementById('about-routes').textContent = data.routes.toLocaleString();
    } catch (e) {
        console.error('Failed to load stats:', e);
    }
}

async function loadStudentInfo() {
    try {
        const { data } = await api('GET', '/api/id');
        document.getElementById('student-info').innerHTML =
            `<dl><dt>Name</dt><dd>${data.name}</dd>` +
            `<dt>Student ID</dt><dd>${data.studentId}</dd>` +
            `<dt>Course</dt><dd>${data.course}</dd>` +
            `<dt>Project</dt><dd>${data.project}</dd></dl>`;
    } catch (e) {
        document.getElementById('student-info').textContent = 'Error loading info';
    }
}

document.querySelectorAll('input[type="text"]').forEach(input => {
    input.addEventListener('keydown', e => {
        if (e.key === 'Enter') {
            const btn = input.closest('.input-group')?.querySelector('button');
            if (btn) btn.click();
        }
    });
});

document.addEventListener('DOMContentLoaded', () => {
    loadStats();
    loadStudentInfo();
});
