import sys
from PyQt5 import QtCore, QtGui, QtWidgets

class MainWindow(QtWidgets.QMainWindow):

    def __init__(self):
        super(MainWindow, self).__init__()

        self.pagelayout = QtWidgets.QVBoxLayout()    #page layout
        self.dbtnlayout = QtWidgets.QHBoxLayout() #layout for the default buttons
        self.addelmtlayout = QtWidgets.QHBoxLayout() #layout for the buttons to add widgets
        self.elmtlayout = QtWidgets.QVBoxLayout()    #layout for the added widgets
        
        #add layouts to the page
        self.pagelayout.addLayout(self.dbtnlayout) 
        self.pagelayout.addLayout(self.addelmtlayout)
        self.pagelayout.addLayout(self.elmtlayout)

        #run, save, load, quit, help button
        btn = QtWidgets.QPushButton(self)
        btn.setText("run")
        btn.clicked.connect(self.run)
        self.dbtnlayout.addWidget(btn)

        btn = QtWidgets.QPushButton(self)
        btn.setText("save")
        btn.clicked.connect(self.save)
        self.dbtnlayout.addWidget(btn)

        btn = QtWidgets.QPushButton(self)
        btn.setText("load")
        btn.clicked.connect(self.load)
        self.dbtnlayout.addWidget(btn)

        btn = QtWidgets.QPushButton(self)
        btn.setText("quit")
        btn.clicked.connect(self.quit)
        self.dbtnlayout.addWidget(btn)

        btn = QtWidgets.QPushButton(self)
        btn.setText("help")
        btn.clicked.connect(self.help)
        self.dbtnlayout.addWidget(btn)

        #add the buttons to add any elements into the gui
        btn = QtWidgets.QPushButton(self)
        btn.setText("make btn")
        btn.clicked.connect(self.makebtn)
        self.addelmtlayout.addWidget(btn)

        btn = QtWidgets.QPushButton(self)
        btn.setText("make radio")
        btn.clicked.connect(self.makerad)
        self.addelmtlayout.addWidget(btn)

        btn = QtWidgets.QPushButton(self)
        btn.setText("make check")
        btn.clicked.connect(self.makecheck)
        self.addelmtlayout.addWidget(btn)

        btn = QtWidgets.QPushButton(self)
        btn.setText("make text")
        btn.clicked.connect(self.maketxt)
        self.addelmtlayout.addWidget(btn)

        btn = QtWidgets.QPushButton(self)
        btn.setText("make slider")
        btn.clicked.connect(self.makeslider)
        self.addelmtlayout.addWidget(btn)

        #add scrollbar
        scroll = QtWidgets.QScrollArea()
        #scroll.setVerticalScrollBarPolicy(QtCore.Qt.ScrollBarAlwaysOn)
        scroll.setWidgetResizable(True)
        
        
        #set the main page layout
        widget = QtWidgets.QWidget()
        widget.setLayout(self.pagelayout)
        scroll.setWidget(widget)
        self.setGeometry(500, 100, 700, 500)
        self.setCentralWidget(scroll)
    
    def run(self):
        print('run')
    
    def save(self):
        print('save')
    
    def load(self):
        print('load')
    
    def quit(self):
        self.close()
        print('quit')

    def help(self):
        print('help')

    def makebtn(self):
        print("button created")
        btn = QtWidgets.QPushButton(self)
        btn.clicked.connect(self.btnClicked)
        self.elmtlayout.addWidget(btn)

    def makerad(self):
        print("radio button created")
        rad = QtWidgets.QRadioButton(self)
        rad.toggled.connect(self.radioClicked)
        self.elmtlayout.addWidget(rad)

    def makecheck(self):
        print("checkbox created")
        check = QtWidgets.QCheckBox(self)
        check.toggled.connect(self.checked)
        self.elmtlayout.addWidget(check)

    def maketxt(self):
        print("text box created")
        txt = QtWidgets.QLineEdit(self)
        self.elmtlayout.addWidget(txt)

    def makeslider(self):
        layout = QtWidgets.QHBoxLayout()
        sliderwidget = QtWidgets.QWidget()
        sliderwidget.setLayout(layout)
        print("slider created")
        #creates a horizontal slider
        slider = QtWidgets.QSlider(self)
        label = QtWidgets.QLabel(self)
        slider.setOrientation(QtCore.Qt.Horizontal)
        slider.setSingleStep(1)
        slider.setPageStep(1)       #moves the slider when clicking or up/down
        slider.setRange(0, 100)
        slider.setValue(30)

        #shows the current position and updates when slider moves
        label.setText(str(slider.value()))
        slider.valueChanged.connect(label.setNum)
        
        layout.addWidget(slider)
        layout.addWidget(label)
        self.elmtlayout.addWidget(sliderwidget)

    def btnClicked(self):
        print("clicked")

    def radioClicked(self):
        rad = self.sender()
        if rad.isChecked():
            print("clicked")
        else:
            print("unclicked")

    def checked(self):
        check = self.sender()
        if check.isChecked():
            print("checked")
        else:
            print("unchecked")

if __name__ == '__main__':
    app = QtWidgets.QApplication(sys.argv)
    w = MainWindow()
    w.show()

    try:
        print('opening window')
        sys.exit(app.exec())
    except SystemExit:
        print('closing window')