import random
def f1(x):
 a1 = 5.0
 a2 = -10.0
 a3 = 10.0
 b1 = 2.0
 return ((a1+a2*x+a3*x**2-b1/x**3)/1000.0)

dp=5.0 #noise's stddev
NP=51

# set seed to ensure replicability
random.seed(1)
t1Name="generic-curve"
t1=newTable(t1Name,3,NP)
for i in range(t1.numRows()):
    xx=float(i+1)
    t1.column(0).setValueAt(i,xx)
    t1.column(1).setValueAt(i,f1(xx)+random.uniform(0,dp))
    t1.column(2).setValueAt(i,10*f1(xx)+random.uniform(0,dp))

g1=plot(t1,'2',1)
layer = g1.activeLayer()
layer.insertCurve(t1,'3',0,1)

symbol = QwtSymbol()
symbol.setStyle(QwtSymbol.Style.Triangle)
symbol.setOutlineColor(QtGui.QColor(Qt.GlobalColor.red))
symbol.setFillColor(QtGui.QColor(Qt.GlobalColor.green))
symbol.setSize(20)
curve=layer.curve(0)
curve.setSymbol(symbol)

# enable RHS curve and axes
layer.curve(1).setXAxis(3) # Top
layer.curve(1).setYAxis(1) # Right
layer.enableAxis(3,True) # xTop
layer.enableAxis(1,True) # xRight

layer.replot()

g1.exportImage("triangle.png")

# OK now extract the pen and brush
pen=symbol.pen()
brush=pen.brush()
assert brush.color()==Qt.GlobalColor.red
brush=symbol.brush()
assert brush.color()==Qt.GlobalColor.green
assert symbol.style()==QwtSymbol.Style.Triangle
assert symbol.size()==QtCore.QSize(20,20)

assert curve.dataSize()==NP
for i in range(t1.numRows()):
    assert curve.sample(i).x()==t1.column(0).valueAt(i)
    assert curve.sample(i).y()==t1.column(1).valueAt(i)
    assert curve.minXValue() < curve.sample(i).x() < curve.maxXValue()
    assert curve.minYValue() < curve.sample(i).y() < curve.maxYValue()

# equality not defined for QwtSymbol
#assert curve.symbol()==symbol
symbol=curve.symbol()
pen=symbol.pen()
brush=pen.brush()
assert brush.color()==Qt.GlobalColor.red
brush=symbol.brush()
assert brush.color()==Qt.GlobalColor.green
assert symbol.style()==QwtSymbol.Style.Triangle
assert symbol.size()==QtCore.QSize(20,20)


pen=curve.pen()
pen.setColor(Qt.GlobalColor.blue)
pen.setWidth(5)
pen.setStyle(Qt.PenStyle.DashLine)
pen.setJoinStyle(Qt.PenJoinStyle.RoundJoin)
pen.setCapStyle(Qt.PenCapStyle.RoundCap)
pen.setMiterLimit(3)
curve.setPen(pen)
assert curve.pen()==pen
pen=curve.pen()
assert pen.color()==Qt.GlobalColor.blue
assert pen.width()==5
assert pen.style()==Qt.PenStyle.DashLine
assert pen.joinStyle()==Qt.PenJoinStyle.RoundJoin
assert pen.capStyle()==Qt.PenCapStyle.RoundCap
assert pen.miterLimit()==3

pen.setDashPattern([3,1,2,1])
pen.setDashOffset(2)
assert pen.dashPattern()==[3,1,2,1]
assert pen.dashOffset()==2
assert pen.style()==Qt.PenStyle.CustomDashLine

brush=curve.brush()
brush.setColor(Qt.GlobalColor.red)
brush.setTransform(QtGui.QTransform(0,1,0,1,10,10))
brush.setStyle(Qt.BrushStyle.BDiagPattern)
curve.setBrush(brush)
assert curve.brush()==brush
brush=curve.brush()
assert brush.color()==Qt.GlobalColor.red
assert brush.transform()==QtGui.QTransform(0,1,0,1,10,10)
assert brush.style()==Qt.BrushStyle.BDiagPattern

curve.setOutlineColor(Qt.GlobalColor.blue)
curve.setFillColor(Qt.GlobalColor.green)
curve.setFillStyle(Qt.BrushStyle.CrossPattern)
assert curve.pen().color()==Qt.GlobalColor.blue
assert curve.brush().color()==Qt.GlobalColor.green
assert curve.brush().style()==Qt.BrushStyle.CrossPattern
app.exit()
