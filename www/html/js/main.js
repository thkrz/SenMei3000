var map = null;
var markers = new Map();

function item(id, text) {
  const li = document.createElement("li");
  const a = document.createElement("a");
  a.href = `station/${id}`;
  a.target = "_blank";
  a.innerHTML = text;
  li.appendChild(a);
  return li;
}

function search() {
  const ul = document.getElementById("rlist");
  ul.replaceChildren();
  const q = document.getElementById("query").value;
  map.eachLayer((l) => {
    if (l instanceof L.Marker)
        l.remove();
  });
  const bounds = L.latLngBounds();
  const isupper = /[A-Z]/.test(q);
  for (const [id, mark] of markers) {
    const s = mark.getTooltip().getContent();
    const cmp = isupper ? s : s.toLowerCase();
    if (q.length === 0 || cmp.includes(q)) {
      mark.addTo(map);
      bounds.extend(mark.getLatLng());
      ul.appendChild(item(id, s));
    }
  }
  map.fitBounds(bounds);
}

function toggleNav() {
  const e = document.getElementById("nav");
  if (e.style.width === "320px") e.style.width = 0;
  else e.style.width = "320px";
}

function hideResults() {
  if (document.getElementById("searchbox").contains(event.relatedTarget))
    return;
  document.getElementById("results").style.display = "none";
}

function showResults() {
  document.getElementById("results").style.display = "block"
  search();
}

function init_map() {
  const uri =
    "https://server.arcgisonline.com/ArcGIS/rest/services/World_Topo_Map/MapServer/tile/{z}/{y}/{x}";
  const copyright =
    "Tiles &copy; Esri &mdash; Esri, DeLorme, NAVTEQ, TomTom, Intermap, iPC, USGS, FAO, NPS, NRCAN, GeoBase, Kadaster NL, Ordnance Survey, Esri Japan, METI, Esri China (Hong Kong), and the GIS User Community";
  map = L.map("map", { minZoom: 3, zoomControl: false });
  L.tileLayer(uri, { attribution: copyright }).addTo(map);
  L.control.scale({ position: "bottomright" }).addTo(map);
}

// main
fetch("http://erdrutsch.com:8000/station")
  .then((r) => r.json())
  .then((l) => {
    init_map();
    let lat = 0;
    let lng = 0;
    const ul = document.getElementById("sbar-list");
    const bounds = L.latLngBounds();
    for (const e of l) {
      let name = e.name;
      if (name)
        name += ", ";
      name += e.id;
      const mark = L.marker([e.lat, e.lng])
        .on("click", () => window.open(`station/${e.id}`, "_blank"))
        .bindTooltip(name)
        .addTo(map);
      markers.set(e.id, mark);
      bounds.extend(mark.getLatLng());
    }
    map.fitBounds(bounds);
  });
