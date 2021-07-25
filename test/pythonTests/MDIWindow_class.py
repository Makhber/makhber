# Script to test the subsections of:
# "API documentation" of the handbook
# available at: https://highperformancecoder.github.io/scidavis-handbook/sec-python.html

## subsection: class MDIWindow (inherits QWidget)

tName = "Table01"
t=newTable(tName,2,10)
t.confirmClose(False)
assert t.name() == tName
tNewName = tName+"New"
t.setName(tNewName)
assert t.name() == tNewName
assert t.windowLabel() == ""
tNewLabel = "a table"
t.setWindowLabel(tNewLabel)
assert t.windowLabel() == tNewLabel
assert t.captionPolicy() == MDIWindow.CaptionPolicy.Both
t.setCaptionPolicy(MDIWindow.CaptionPolicy.Name)
## default captionPolicy is 2: show both name and label

# t.folder() will return something like 
# <makhber.Folder object at 0xaec9c8e4>
# But the expected output would be a folder name...

# I don't know how to test "confirmClose(boolean)" because if I set it as True and try to
# close the object there will be no error message,  but a confirmation window will open. 
# Otherwise it just closes...

t.setCell(1,1,10.0)
t2 = t.clone()
t2.confirmClose(False)
assert t2.captionPolicy() == MDIWindow.CaptionPolicy.Name
assert t2.numRows() == t.numRows()
assert t2.numCols() == t.numCols()
assert t2.cell(1,1) == t.cell(1,1)

# Do some tests of "class MDIWindow" for graph, matrix and note
## graph...
gName = "Graph01"
g=newGraph(gName)
g.confirmClose(False)
assert g.name() == gName
gNewName = gName+"New"
g.setName(gNewName)
assert g.name() == gNewName
assert g.windowLabel() == ""
gNewLabel = "a graph"
g.setWindowLabel(gNewLabel)
assert g.windowLabel() == gNewLabel
assert g.captionPolicy() == MDIWindow.CaptionPolicy.Both
g.setCaptionPolicy(MDIWindow.CaptionPolicy.Name)
g2 = g.clone()
assert g2.captionPolicy() == MDIWindow.CaptionPolicy.Name
g2.confirmClose(False)

# matrix...
mName = "Matrix01"
m=newMatrix(mName,10,10)
m.confirmClose(False)
assert m.name() == mName
mNewName = mName+"New"
m.setName(mNewName)
assert m.name() == mNewName
assert m.windowLabel() == ""
mNewLabel = "a matrix"
m.setWindowLabel(mNewLabel)
assert m.windowLabel() == mNewLabel
assert m.captionPolicy() == MDIWindow.CaptionPolicy.Both
m.setCaptionPolicy(MDIWindow.CaptionPolicy.Name)
m2 = m.clone()
assert m2.captionPolicy() == MDIWindow.CaptionPolicy.Name
m2.confirmClose(False)

# note...
nName = "Note01"
n=newNote(nName)
n.confirmClose(False)
assert n.name() == nName
nNewName = nName+"New"
n.setName(nNewName)
assert n.name() == nNewName
assert n.windowLabel() == ""
nNewLabel = "a note"
n.setWindowLabel(nNewLabel)
assert n.windowLabel() == nNewLabel
assert n.captionPolicy() == MDIWindow.CaptionPolicy.Both
n.setCaptionPolicy(MDIWindow.CaptionPolicy.Name)
n2 = n.clone()
assert n2.captionPolicy() == MDIWindow.CaptionPolicy.Name
n2.confirmClose(False)
app.exit()
