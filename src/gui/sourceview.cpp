#include <sourceview.h>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

SourceView::SourceView(QWidget* parent)
{
	init();
}

void
SourceView::init()
{
	uriEdit = new QLineEdit(this);
	usernameEdit = new QLineEdit(this);
	passwordEdit = new QLineEdit(this);
	outputDirEdit = new QLineEdit(this);
	brandComboBox = new QComboBox(this);
	resolutionComboBox = new QComboBox(this);
	browseBtn = new QPushButton("Browse", this);	
	QVBoxLayout* mainLayout = new QVBoxLayout();

	QHBoxLayout* h1 = new QHBoxLayout();
	h1->addWidget(new QLabel("URI: ", this));
	h1->addWidget(uriEdit);
	mainLayout->addLayout(h1);

	QHBoxLayout* h2 = new QHBoxLayout();
	h2->addWidget(new QLabel("Username: ", this));
	h2->addWidget(usernameEdit);
	mainLayout->addLayout(h2);

	QHBoxLayout* h3 = new QHBoxLayout();
	h3->addWidget(new QLabel("Password: ", this));
	h3->addWidget(passwordEdit);
	mainLayout->addLayout(h3);

	QHBoxLayout* h4 = new QHBoxLayout();
	h4->addWidget(new QLabel("Camera Brand: ", this));
	h4->addWidget(brandComboBox);
	mainLayout->addLayout(h4);

	QHBoxLayout* h5 = new QHBoxLayout();
	h5->addWidget(new QLabel("Resolution: ", this));
	h5->addWidget(resolutionComboBox);
	mainLayout->addLayout(h5);

	QHBoxLayout* h6 = new QHBoxLayout();
	h6->addWidget(new QLabel("Video Output Directory: ", this));
	h6->addWidget(outputDirEdit);
	h6->addWidget(browseBtn);
	mainLayout->addLayout(h6);

	setLayout(mainLayout);
}


