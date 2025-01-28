var map = L.map("map", { minZoom: 3, zoomControl: false }).setView([0, 0], 4);
L.tileLayer(
  "https://server.arcgisonline.com/ArcGIS/rest/services/World_Topo_Map/MapServer/tile/{z}/{y}/{x}",
  {
    attribution:
      "Tiles &copy; Esri &mdash; Esri, DeLorme, NAVTEQ, TomTom, Intermap, iPC, USGS, FAO, NPS, NRCAN, GeoBase, Kadaster NL, Ordnance Survey, Esri Japan, METI, Esri China (Hong Kong), and the GIS User Community",
  },
).addTo(map);
L.control.scale().addTo(map);

Dygraph.onDOMready(function onDOMready() {
  g = new Dygraph(
    // containing div
    document.getElementById("m1"),

    // CSV or path to a CSV file.
    "Date,Temperature\n" +
      "2008-05-07,75\n" +
      "2008-05-08,70\n" +
      "2008-05-09,80\n",
    { width: "auto" },
  );

  g2 = new Dygraph(
    // containing div
    document.getElementById("m2"),

    // CSV or path to a CSV file.
    "Date,Temperature\n" +
      "2008-05-07,75\n" +
      "2008-05-08,70\n" +
      "2008-05-09,80\n",
    { width: "auto" },
  );

  g3 = new Dygraph(
    // containing div
    document.getElementById("g1"),

    // CSV or path to a CSV file.
    "Date,Temperature\n" +
      "2008-05-07,75\n" +
      "2008-05-08,70\n" +
      "2008-05-09,80\n",
    { width: "auto" },
  );
  g4 = new Dygraph(
    // containing div
    document.getElementById("g2"),

    // CSV or path to a CSV file.
    "Date,Temperature\n" +
      "2008-05-07,75\n" +
      "2008-05-08,70\n" +
      "2008-05-09,80\n",
    { width: "auto" },
  );
  g5 = new Dygraph(
    // containing div
    document.getElementById("g3"),

    // CSV or path to a CSV file.
    "Date,Temperature\n" +
      "2008-05-07,75\n" +
      "2008-05-08,70\n" +
      "2008-05-09,80\n",
    { width: "auto" },
  );
  g6 = new Dygraph(
    // containing div
    document.getElementById("g4"),

    // CSV or path to a CSV file.
    "Date,Temperature\n" +
      "2008-05-07,75\n" +
      "2008-05-08,70\n" +
      "2008-05-09,80\n",
    { width: "auto" },
  );
});
