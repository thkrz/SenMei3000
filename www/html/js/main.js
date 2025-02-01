function createConfig(dat, tab) {
  const dl = document.getElementById("sensors");
  dl.replaceChildren();
  for (const [k, v] of Object.entries(dat.config)) {
    const dt = document.createElement("dt");
    dt.style = "text-transform: none; font-weight: 500";
    dt.appendChild(document.createTextNode(k));

    const dd = document.createElement("dd");

    let label = document.createElement("label");
    label.appendChild(document.createTextNode("Sensor:"));
    const sel = document.createElement("select");
    sel.name = k;
    for (let i = 0; i < tab.length; i++) {
      const opt = document.createElement("option");
      opt.value = i;
      opt.appendChild(document.createTextNode(tab[i].name));
      sel.appendChild(opt);
    }
    sel.selectedIndex = dat.config[k].sensor;
    label.appendChild(sel);

    dd.appendChild(label);

    label = document.createElement("label");
    label.appendChild(document.createTextNode("Label:"));
    const input = document.createElement("input");
    input.value = dat.config[k].label;
    input.name = k;
    label.append(input);

    dd.appendChild(label);

    dl.appendChild(dt);
    dl.appendChild(dd);
  }
}

function createGraphs(c, x, y) {
  c.replaceChildren();
  for (const [_, ser] of Object.entries(y)) {
    const g = document.createElement("div");
    c.appendChild(g);
    opts = {
      title: ser.title,
      width: "auto",
      labels: ["Date"].concat(ser.labels),
      rollPeriod: Math.floor(x.length / 100),
      showRoller: true,
      series: {},
    };
    if (ser.length > 1) {
      opts.series[ser.labels[0]] = { color: "#0000ff" };
      opts.series[ser.labels[1]] = { color: "#ff0000", axis: "y2" };
    } else {
      opts.series[ser.labels[0]] = { color: "#00ff00" };
    }
    new Dygraph(
      g,
      x.map((e, i) => {
        return [new Date(e)].concat(ser.data[i]);
      }),
      opts
    );
  }
}

function hide() {
  document.getElementById("dform").style.visibility = "hidden";
}

function show(sid) {
  Promise.all([
    fetch("http://127.0.0.1:8000/station/" + sid).then((r) => r.json()),
    fetch("http://127.0.0.1:8000/sensor").then((r) => r.json()),
  ]).then(([series, meta]) => {
    document
      .getElementById("sid")
      .replaceChildren(document.createTextNode(series.id));
    const form = document.getElementById("meta");
    for (const [k, v] of Object.entries(series)) {
      const field = form.querySelector(`input[name="${k}"]`);
      field && (field.value = v);
    }
    createConfig(series, meta);
    createGraphs(document.getElementById("health"), series.t, series.h);
    createGraphs(document.getElementById("data"), series.t, series.s);
  });
  document.getElementById("dform").style.visibility = "visible";
  return false;
}

async function submit(form) {
  const o = {
    name: form.querySelector('input[name="name"]').value,
    lat: parseFloat(form.querySelector('input[name="lat"]').value || 0),
    lng: parseFloat(form.querySelector('input[name="lng"]').value || 0),
    note: form.querySelector('input[name="note"]').value,
  };
  const items = document.getElementById("sensors").children;
  const config = {};
  for (let i = 0; i < items.length; i += 2) {
    const k = items[i].childNodes[0].nodeValue;
    config[k] = {
      sensor: parseInt(
        items[i + 1].querySelector(`select[name="${k}"]`).value || -1
      ),
      label: items[i + 1].querySelector(`input[name="${k}"]`).value,
    };
  }
  o["config"] = config;
  const sid = document.getElementById("sid").childNodes[0].nodeValue;
  await fetch(`http://127.0.0.1:8000/station/${sid}/update`, {
    method: "POST",
    headers: { Accept: "application/json", "Content-Type": "application/json" },
    body: JSON.stringify(o),
  });
}

function init_map() {
  const uri =
    "https://server.arcgisonline.com/ArcGIS/rest/services/World_Topo_Map/MapServer/tile/{z}/{y}/{x}";
  const copyright =
    "Tiles &copy; Esri &mdash; Esri, DeLorme, NAVTEQ, TomTom, Intermap, iPC, USGS, FAO, NPS, NRCAN, GeoBase, Kadaster NL, Ordnance Survey, Esri Japan, METI, Esri China (Hong Kong), and the GIS User Community";
  let map = L.map("map", { minZoom: 3, zoomControl: false });
  L.tileLayer(uri, { attribution: copyright }).addTo(map);
  L.control.scale().addTo(map);
  return map;
}

function search_item(e) {
  const li = document.createElement("li");
  const a = document.createElement("a");
  a.href = "#";
  let s = e.name;
  if (s !== "") s += ", ";
  li.innerHTML = `${s}<span class='addendum'>${e.id}</span>`;
  a.appendChild(li);
  return a;
}

const form = document.getElementById("meta");
form.addEventListener("submit", (e) => {
  e.preventDefault();
  submit(form);
});

fetch("http://127.0.0.1:8000/station")
  .then((r) => r.json())
  .then((l) => {
    var map = init_map();
    let lat = 0;
    let lng = 0;
    const ul = document.getElementById("search-results");
    for (let i = 0; i < l.length; i++) {
      const latlng = [l[i].lat, l[i].lng];
      const a = search_item(l[i]);
      a.addEventListener(
        "click",
        (ev) => {
          show(l[i].id);
          map.setView(latlng, 12);
        },
        false
      );
      ul.appendChild(a);
      L.marker(latlng)
        .addTo(map)
        .on("click", (ev) => show(l[i].id))
        .bindTooltip(a.lastChild.innerHTML);
      lat += l[i].lat;
      lng += l[i].lng;
    }
    lat /= l.length;
    lng /= l.length;
    map.setView([lat, lng], 9);
  });
