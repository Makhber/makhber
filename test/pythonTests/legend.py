graph=newGraph()
layer = graph.activeLayer()
legend = layer.newLegend("hello world")
legend.setBackgroundColor(Qt.GlobalColor.green)
legend.setTextColor(Qt.GlobalColor.darkBlue)
legend.setFrameStyle(2)
legend.setFont(QtGui.QFont("Arial",14,QtGui.QFont.Weight.Bold))
legend.setOriginCoord(400,400)
layer.replot()
graph.exportImage("legend.png");
app.exit()

