function wnd_close() {
  document.getElementById("dform").style.visibility = "hidden";
}

function init_map() {
  let map = L.map("map", { minZoom: 3, zoomControl: false });
  L.tileLayer(
    "https://server.arcgisonline.com/ArcGIS/rest/services/World_Topo_Map/MapServer/tile/{z}/{y}/{x}",
    {
      attribution:
        "Tiles &copy; Esri &mdash; Esri, DeLorme, NAVTEQ, TomTom, Intermap, iPC, USGS, FAO, NPS, NRCAN, GeoBase, Kadaster NL, Ordnance Survey, Esri Japan, METI, Esri China (Hong Kong), and the GIS User Community",
    }
  ).addTo(map);
  L.control.scale().addTo(map);
  return map;
}

function search_item(e) {
  const li = document.createElement("li");
  const a = document.createElement("a");
  a.href = "#" + e.id;
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

Dygraph.onDOMready(function onDOMready() {
  g = new Dygraph(
    // containing div
    document.getElementById("m1"),

    // CSV or path to a CSV file.
    "Date,Temperature\n" +
      "2008-05-07,75\n" +
      "2008-05-08,70\n" +
      "2008-05-09,80\n",
    { width: "auto" }
  );

  g2 = new Dygraph(
    // containing div
    document.getElementById("m2"),

    // CSV or path to a CSV file.
    "Date,Temperature\n" +
      "2008-05-07,75\n" +
      "2008-05-08,70\n" +
      "2008-05-09,80\n",
    { width: "auto" }
  );

  g3 = new Dygraph(
    // containing div
    document.getElementById("g1"),

    // CSV or path to a CSV file.
    "Date,Temperature\n" +
      "2008-05-07,75\n" +
      "2008-05-08,70\n" +
      "2008-05-09,80\n",
    { width: "auto" }
  );
  g4 = new Dygraph(
    // containing div
    document.getElementById("g2"),

    // CSV or path to a CSV file.
    "Date,Temperature\n" +
      "2008-05-07,75\n" +
      "2008-05-08,70\n" +
      "2008-05-09,80\n",
    { width: "auto" }
  );
  g5 = new Dygraph(
    // containing div
    document.getElementById("g3"),

    // CSV or path to a CSV file.
    "Date,Temperature\n" +
      "2008-05-07,75\n" +
      "2008-05-08,70\n" +
      "2008-05-09,80\n",
    { width: "auto" }
  );
  g6 = new Dygraph(
    // containing div
    document.getElementById("g4"),

    // CSV or path to a CSV file.
    "Date,Temperature\n" +
      "2008-05-07,75\n" +
      "2008-05-08,70\n" +
      "2008-05-09,80\n",
    { width: "auto" }
  );
});
