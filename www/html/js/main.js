function createSchemaNodes(dat, tab) {
  const dl = document.getElementById("sensors");
  dl.replaceChildren();
  for (const [k, v] of Object.entries(dat.schema)) {
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
    sel.selectedIndex = dat.schema[k].type;
    label.appendChild(sel);

    dd.appendChild(label);

    label = document.createElement("label");
    label.appendChild(document.createTextNode("Label:"));
    const input = document.createElement("input");
    input.value = dat.schema[k].label;
    input.name = k;
    label.append(input);

    dd.appendChild(label);

    dl.appendChild(dt);
    dl.appendChild(dd);
  }
}

function hide() {
  document.getElementById("dform").style.visibility = "hidden";
}

function show(sid) {
  fetch("http://127.0.0.1:8000/station/" + sid)
    .then((r) => r.json())
    .then((dt) => {
      document
        .getElementById("sid")
        .replaceChildren(document.createTextNode(dt.dat.id));
      const form = document.getElementById("meta");
      for (const [k, v] of Object.entries(dt.dat)) {
        const field = form.querySelector(`input[name="${k}"]`);
        field && (field.value = v);
      }
      createSchemaNodes(dt.dat, dt.tab);

      // const div = document.getElementById("health");
      // createGraph(
      //   div,
      //   o.t.map((e, i) => {
      //     return [new Date(e), o.s["#"][i]];
      //   }),
      //   ["Date", "Voltage"]
      // );
      // createGraph(
      //   div,
      //   o.t.map((e, i) => {
      //     return [new Date(e), o.s["*"][i][0], o.s["*"][i][1]];
      //   }),
      //   ["Date", "Temperature", "Humidity"]
      // );
    });
  document.getElementById("dform").style.visibility = "visible";
  return false;
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
