import sys
from PyQt5.QtWidgets import *
from PyQt5.QtGui import QIcon
from PyQt5.QtCore import pyqtSlot
from PyQt5.QtGui import *

class App(QWidget):

    def __init__(self):
        super().__init__()
        #sets window dimensions
        self.setGeometry(50,50,320,200)
        self.setWindowTitle("PyQt5 Example")

        #Adds text. Must use a label to add the text
        textLabel = QLabel(self)
        textLabel.setText("Hello World!")
        #move(x-coor, y-corr): positions the element in relation to the display window
        textLabel.move(110,85)

        #makes a button
        button1 = QPushButton(self)
        button1.setText("click!")
        button1.move(0,0)

        #connects the clicking of the button to some function
        button1.clicked.connect(self.button_clicked)

        #sets the window that pops up and displays it
        #setGeometry(x-coor, y-coor, width, height)
        self.setGeometry(50,50,320,200)
        self.setWindowTitle("PyQt5 Example")

        #browse files button
        filebutton = QPushButton(self)
        filebutton.setText("file browse")
        filebutton.move(100, 0)

        filebutton.clicked.connect(self.browse_files)

        #makes radio buttons
        layout = QGridLayout()  #layout managers can help with the layout of elements
        self.setLayout(layout)
        radiobutton = QRadioButton("button1")
        radiobutton.setChecked(True)
        radiobutton.name = "button1"
        radiobutton.toggled.connect(self.onClicked)
        layout.addWidget(radiobutton, 0, 50)

        radiobutton = QRadioButton("button2")
        radiobutton.name = "button2"
        radiobutton.toggled.connect(self.onClicked)
        layout.addWidget(radiobutton, 0, 100)

    def onClicked(self):
        radioButton = self.sender()
        if radioButton.isChecked():
            print("Clicked %s" % (radioButton.name))


    def button_clicked(self):
        print("click")

        #makes a popup when clicked
        msgBox = QMessageBox()
        msgBox.setText("pop up window")
        msgBox.setWindowTitle("QMessageBox Example")
        msgBox.setStandardButtons(QMessageBox.Ok | QMessageBox.Cancel)
        returnValue = msgBox.exec()

        if returnValue == QMessageBox.Ok:
            print('OK clicked')

    def browse_files(self):
        #dir_ = QFileDialog.getExistingDirectory(None, 'Select a folder:', 'C:\\', QFileDialog.ShowDirsOnly)
        # folder = str(QFileDialog.getExistingDirectory(None, "Select Directory"))
        # print(folder)
        file = QFileDialog.getOpenFileNames(self, "Select File", "")
        print(file)

if __name__ == '__main__':
    app = QApplication(sys.argv)
    ex = App()
    ex.show()

    try:
        print('opening window')
        sys.exit(app.exec_())
    except SystemExit:
        print('closing window')