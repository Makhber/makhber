/***************************************************************************
    File                 : ImageExportDialog.cpp
    Project              : Makhber
    --------------------------------------------------------------------
    Copyright            : (C) 2006,2007 by Ion Vasilief,
                           Tilman Benkert, Knut Franke
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net,
                           knut.franke*gmx.de
    Description          : QFileDialog extended with options for image export

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/
#include "ImageExportDialog.h"

#include "core/ApplicationWindow.h"

#include <QStackedWidget>
#include <QImageWriter>
#include <QGroupBox>
#include <QPushButton>
#include <QGridLayout>
#include <QPrinter>
#include <QLabel>
#include <QComboBox>
#include <QCloseEvent>

ImageExportDialog::ImageExportDialog(QWidget *parent, bool vector_options, bool extended,
                                     Qt::WindowFlags flags)
    : ExtensibleFileDialog(parent, extended, flags)
{
    setWindowTitle(tr("Choose a filename to save under"));
    setAcceptMode(QFileDialog::AcceptSave);

    QList<QByteArray> list = QImageWriter::supportedImageFormats();
    list << "EPS";
    list << "PS";
    list << "PDF";
    list << "SVG";

    QStringList filters;
    for (int i = 0; i < list.count(); i++)
        filters << "*." + list[i].toLower();

    filters.sort();
    setNameFilters(filters);
    setFileMode(QFileDialog::AnyFile);

    initAdvancedOptions();
    d_vector_options->setEnabled(vector_options);
    setExtensionWidget(d_advanced_options);

    connect(this, SIGNAL(filterSelected(const QString &)), this,
            SLOT(updateAdvancedOptions(const QString &)));
    QList<QComboBox *> combo_boxes = findChildren<QComboBox *>();
    if (combo_boxes.size() >= 2)
        connect(combo_boxes[1], SIGNAL(currentIndexChanged(const QString &)), this,
                SLOT(updateAdvancedOptions(const QString &)));
    updateAdvancedOptions(selectedNameFilter());
}

void ImageExportDialog::initAdvancedOptions()
{
    auto *app = dynamic_cast<ApplicationWindow *>(this->parent());
    d_advanced_options = new QStackedWidget();

    d_vector_options = new QGroupBox();
    auto *vector_layout = new QGridLayout(d_vector_options);
    d_advanced_options->addWidget(d_vector_options);

    vector_layout->addWidget(new QLabel(tr("Resolution (DPI)")), 1, 0);
    d_resolution = new QSpinBox();
    d_resolution->setRange(0, 1000);
    d_resolution->setValue(app->d_export_resolution);
    vector_layout->addWidget(d_resolution, 1, 1);
    d_resolution->setEnabled(false); // FIXME temporary disabled

    d_color = new QCheckBox();
    d_color->setText(tr("Export in &color"));
    d_color->setChecked(app->d_export_color);
    vector_layout->addWidget(d_color, 2, 0, 1, 2);

    d_standard_page = new QCheckBox();
    d_standard_page->setText(tr("Export to &standard page size"));
    vector_layout->addWidget(d_standard_page, 3, 0, 1, 2);

    d_box_page_size = new QComboBox();
    d_box_page_size->addItem(tr("A0 - 841 x 1189 mm"), QVariant(QPageSize::A0));
    d_box_page_size->addItem(tr("A1 - 594 x 841 mm"), QVariant(QPageSize::A1));
    d_box_page_size->addItem(tr("A2 - 420 x 594 mm"), QVariant(QPageSize::A2));
    d_box_page_size->addItem(tr("A3 - 297 x 420 mm"), QVariant(QPageSize::A3));
    d_box_page_size->addItem(tr("A4 - 210 x 297 mm"), QVariant(QPageSize::A4));
    d_box_page_size->addItem(tr("A5 - 148 x 210 mm"), QVariant(QPageSize::A5));
    d_box_page_size->addItem(tr("A6 - 105 x 148 mm"), QVariant(QPageSize::A6));
    d_box_page_size->addItem(tr("A7 - 74 x 105 mm"), QVariant(QPageSize::A7));
    d_box_page_size->addItem(tr("A8 - 52 x 74 mm"), QVariant(QPageSize::A8));
    d_box_page_size->addItem(tr("A9 - 37 x 52 mm"), QVariant(QPageSize::A9));
    d_box_page_size->addItem(tr("B0 - 1030 x 1456 mm"), QVariant(QPageSize::B0));
    d_box_page_size->addItem(tr("B1 - 728 x 1030 mm"), QVariant(QPageSize::B1));
    d_box_page_size->addItem(tr("B2 - 515 x 728 mm"), QVariant(QPageSize::B2));
    d_box_page_size->addItem(tr("B3 - 364 x 515 mm"), QVariant(QPageSize::B3));
    d_box_page_size->addItem(tr("B4 - 257 x 364 mm"), QVariant(QPageSize::B4));
    d_box_page_size->addItem(tr("B5 - 182 x 257 mm"), QVariant(QPageSize::B5));
    d_box_page_size->addItem(tr("B6 - 128 x 182 mm"), QVariant(QPageSize::B6));
    d_box_page_size->addItem(tr("B7 - 91 x 128 mm"), QVariant(QPageSize::B7));
    d_box_page_size->addItem(tr("B8 - 64 x 91 mm"), QVariant(QPageSize::B8));
    d_box_page_size->addItem(tr("B9 - 45 x 64 mm"), QVariant(QPageSize::B9));
    d_box_page_size->addItem(tr("B10 - 32 x 45 mm"), QVariant(QPageSize::B10));
    d_box_page_size->addItem(tr("C5E - 163 x 226 mm"), QVariant(QPageSize::C5E));
    d_box_page_size->addItem(tr("Comm10E - 105 x 241 mm"), QVariant(QPageSize::Comm10E));
    d_box_page_size->addItem(tr("DLE - 110 x 220 mm"), QVariant(QPageSize::DLE));
    d_box_page_size->addItem(tr("Executive - 191 x 254 mm"), QVariant(QPageSize::Executive));
    d_box_page_size->addItem(tr("Folio 210 x 330 mm"), QVariant(QPageSize::Folio));
    d_box_page_size->addItem(tr("Ledger 432 x 279 mm"), QVariant(QPageSize::Ledger));
    d_box_page_size->addItem(tr("Legal 216 x 356 mm"), QVariant(QPageSize::Legal));
    d_box_page_size->addItem(tr("Letter 216 x 279 mm"), QVariant(QPageSize::Letter));
    d_box_page_size->addItem(tr("Tabloid 279 x 432 mm"), QVariant(QPageSize::Tabloid));

    vector_layout->addWidget(new QLabel(tr("Page size")), 4, 0);
    setPageSize(QPageSize((QPageSize::PageSizeId)app->d_export_vector_size));
    d_standard_page->setChecked(app->d_export_vector_size != QPageSize::Custom);
    d_box_page_size->setEnabled(app->d_export_vector_size != QPageSize::Custom);
    vector_layout->addWidget(d_box_page_size, 4, 1, 1, 2);

    vector_layout->addWidget(new QLabel(tr("Orientation")), 5, 0);
    d_box_page_orientation = new QComboBox();
    d_box_page_orientation->addItem(tr("Portrait", "page orientation"),
                                    QVariant(QPageLayout::Portrait));
    d_box_page_orientation->addItem(tr("Landscape", "page orientation"),
                                    QVariant(QPageLayout::Landscape));
    d_box_page_orientation->setCurrentIndex(app->d_export_orientation);
    d_box_page_orientation->setEnabled(app->d_export_vector_size != QPageSize::Custom);
    vector_layout->addWidget(d_box_page_orientation, 5, 1, 1, 2);

    connect(d_standard_page, SIGNAL(toggled(bool)), d_box_page_size, SLOT(setEnabled(bool)));
    connect(d_standard_page, SIGNAL(toggled(bool)), d_box_page_orientation, SLOT(setEnabled(bool)));

    d_keep_aspect = new QCheckBox();
    d_keep_aspect->setText(tr("&Keep aspect ratio"));
    d_keep_aspect->setChecked(app->d_keep_plot_aspect);
    vector_layout->addWidget(d_keep_aspect, 6, 0, 1, 2);

    d_raster_options = new QGroupBox();
    auto *raster_layout = new QGridLayout(d_raster_options);
    d_advanced_options->addWidget(d_raster_options);

    raster_layout->addWidget(new QLabel(tr("Image quality")), 1, 0);
    d_quality = new QSpinBox();
    d_quality->setRange(1, 100);
    d_quality->setValue(app->d_export_quality);
    raster_layout->addWidget(d_quality, 1, 1);
}

void ImageExportDialog::updateAdvancedOptions(const QString &filter)
{
    if (filter.contains("*.svg")) {
        d_extension_toggle->setChecked(false);
        d_extension_toggle->setEnabled(false);
        return;
    }
    d_extension_toggle->setEnabled(true);
    if (filter.contains("*.eps") || filter.contains("*.ps") || filter.contains("*.pdf"))
        d_advanced_options->setCurrentIndex(0);
    else {
        d_advanced_options->setCurrentIndex(1);
    }
}

void ImageExportDialog::closeEvent(QCloseEvent *e)
{
    auto *app = dynamic_cast<ApplicationWindow *>(this->parent());
    if (app) {
        app->d_extended_export_dialog = this->isExtended();
        app->d_image_export_filter = this->selectedNameFilter();
        app->d_export_quality = d_quality->value();

        app->d_export_resolution = d_resolution->value();
        app->d_export_color = d_color->isChecked();
        app->d_export_vector_size = (int)pageSize().id();
        app->d_keep_plot_aspect = d_keep_aspect->isChecked();
        app->d_export_orientation = d_box_page_orientation->currentIndex();
    }

    e->accept();
}

QPageSize ImageExportDialog::pageSize() const
{
    if (!d_standard_page->isChecked())
        return QPageSize(QPageSize::Custom);

    QPageSize size;
    switch (d_box_page_size->currentIndex()) {
    case -1:
        size = QPageSize(QPageSize::A4);
        break;
    default:
        size = QPageSize(
                (QPageSize::PageSizeId)d_box_page_size->itemData(d_box_page_size->currentIndex())
                        .toInt());
        break;
    }
    return size;
}

QPageLayout::Orientation ImageExportDialog::pageOrientation() const
{
    QPageLayout::Orientation orientation {};
    switch (d_box_page_orientation->currentIndex()) {
    case -1:
        orientation = QPageLayout::Portrait;
        break;
    default:
        orientation = (QPageLayout::Orientation)d_box_page_orientation
                              ->itemData(d_box_page_orientation->currentIndex())
                              .toInt();
        break;
    }
    return orientation;
}

void ImageExportDialog::setPageSize(QPageSize size)
{
    if (size == QPageSize(QPageSize::Custom))
        return;

    int index = d_box_page_size->findData(QVariant(size.id()));
    if (index != -1)
        d_box_page_size->setCurrentIndex(index);
    else {
        index = d_box_page_size->findData(QVariant(QPageSize::A4));
        Q_ASSERT(index != -1);
        d_box_page_size->setCurrentIndex(index);
    }
}

void ImageExportDialog::setOrientation(QPageLayout::Orientation orientation)
{
    int index = d_box_page_orientation->findData(QVariant(orientation));
    if (index != -1)
        d_box_page_orientation->setCurrentIndex(index);
    else
        d_box_page_orientation->setCurrentIndex(0);
}

void ImageExportDialog::selectFilter(const QString &filter)
{
    QFileDialog::selectNameFilter(filter);
    updateAdvancedOptions(filter);
}
