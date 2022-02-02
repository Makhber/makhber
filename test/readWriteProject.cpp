
#include "utils.h"

#include "testPaintDevice.h"
#include "ApplicationWindowTest.h"
#include "plot2D/MultiLayer.h"
#include "plot3D/Graph3D.h"
#include "scripting/Note.h"

#include <QMdiArea>

#include <iostream>
#include <fstream>
#include <memory>
#include <stdexcept>

TEST_F(ApplicationWindowTest, readWriteProject)
{
    std::unique_ptr<ApplicationWindow> app(open("testProject.mkbr"));
    EXPECT_TRUE(app.get());
    app->saveFolder(app->projectFolder(), "testProject1.mkbr");
    std::unique_ptr<ApplicationWindow> app1(open("testProject1.mkbr"));
    // TODO check that app1 is the same as app?
    EXPECT_TRUE(app1.get());
}

TEST_F(ApplicationWindowTest, exportTestProject)
{
    {
        std::unique_ptr<ApplicationWindow> app(open("testProject.mkbr"));
        EXPECT_TRUE(app.get());
        QList<MyWidget *> windows = app->windowsList();
        for (auto i : windows)
            if (auto w = dynamic_cast<MultiLayer *>(i)) {
                w->exportSVG(i->windowTitle() + ".svg");
                w->exportImage(i->windowTitle() + ".png");
            }
    }

    {
        std::unique_ptr<ApplicationWindow> app(open("3dplot.mkbr"));
        EXPECT_TRUE(app.get());
        QList<MyWidget *> windows = app->windowsList();
        for (auto i : windows)
            if (auto w = dynamic_cast<Graph3D *>(i))
                w->exportImage(i->windowTitle() + ".png");
    }
}
#ifdef ORIGIN_IMPORT
// checks that the large file Origin import problem (ticket #238) remains fixed
TEST_F(ApplicationWindowTest, largeOriginImport)
{
    std::unique_ptr<ApplicationWindow> app(importOPJ("Histo.opj"));
    EXPECT_TRUE(app.get());
}
#endif

TEST_F(ApplicationWindowTest, note)
{
    Note &note = *newNote();
    activateSubWindow(&note);
}
TEST_F(ApplicationWindowTest, convertMatrix)
{
    std::unique_ptr<ApplicationWindow> app(open("testProject.mkbr"));
    EXPECT_TRUE(app.get());
    QList<MyWidget *> windows = app->windowsList();
    for (auto i : windows)
        if (auto table = dynamic_cast<Table *>(i)) {
            EXPECT_TRUE(table->name() == "Table1");
            EXPECT_EQ(30, table->numRows());
            EXPECT_EQ(2, table->numCols());
            app->activateSubWindow(table);
            EXPECT_TRUE(table == app->d_workspace.activeSubWindow());
            app->convertTableToMatrix();
            // active window should switch to a matrix
            auto *matrix = dynamic_cast<Matrix *>(app->d_workspace.activeSubWindow());
            EXPECT_TRUE(matrix);
            EXPECT_EQ(matrix->numRows(), table->numRows());
            EXPECT_EQ(matrix->numCols(), table->numCols());
            for (int r = 0; r < matrix->numRows(); ++r)
                for (int c = 0; c < matrix->numCols(); ++c)
                    EXPECT_NEAR(matrix->cell(r, c), table->cell(r, c), 1e-5);
        }
}
