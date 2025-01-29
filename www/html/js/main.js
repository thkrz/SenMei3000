function wnd_close() {
  document.getElementById("dform").style.visibility = "hidden";
}

function wnd_open(sid) {
  fetch("http://127.0.0.1:8000/station/" + sid)
    .then((r) => r.json())
    .then((o) => {
      const ts = o.timeseries;
      const x = [];
      for (let i = 0; i < ts.length; i++) {
        x.push([new Date(ts[i].date), ts[i].bat]);
      }
      new Dygraph(document.getElementById("gbat"), x, { width: "auto", labels: ["Date", "V"] });
    });
  document.getElementById("dform").style.visibility = "visible";
  return false;
}

function init_map() {
  let map = L.map("map", { minZoom: 3, zoomControl: false });
  L.tileLayer(
    "https://server.arcgisonline.com/ArcGIS/rest/services/World_Topo_Map/MapServer/tile/{z}/{y}/{x}",
    {
      attribution:
        "Tiles &copy; Esri &mdash; Esri, DeLorme, NAVTEQ, TomTom, Intermap, iPC, USGS, FAO, NPS, NRCAN, GeoBase, Kadaster NL, Ordnance Survey, Esri Japan, METI, Esri China (Hong Kong), and the GIS User Community",
    },
  ).addTo(map);
  L.control.scale().addTo(map);
  return map;
}

function search_item(e) {
  const li = document.createElement("li");
  const a = document.createElement("a");
  a.href = "#";
  a.addEventListener("click", (ev) => wnd_open(e.id), false);
  li.innerHTML = e.name + ", <span class='addendum'>" + e.id + "</span>";
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
      L.marker([l[i].lat, l[i].lng]).addTo(map);
      lat += l[i].lat;
      lng += l[i].lng;
      ul.appendChild(search_item(l[i]));
    }
    lat /= l.length;
    lng /= l.length;
    map.setView([lat, lng], 9);
  });
