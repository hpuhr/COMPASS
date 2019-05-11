#ifndef ASTERIXCONFIGWIDGET_H
#define ASTERIXCONFIGWIDGET_H

#include <QWidget>
#include <memory>

#include <jasterix/jasterix.h>

class ASTERIXImporterTask;

class QVBoxLayout;
class QGridLayout;
class QComboBox;
class QPushButton;
class ASTERIXFramingComboBox;

class ASTERIXConfigWidget : public QWidget
{
    Q_OBJECT

signals:

public slots:
    void framingChangedSlot();
    void framingEditSlot();

    void categoryCheckedSlot ();
    void editionChangedSlot(const std::string& cat_str, const std::string& ed_str);
    void categoryEditSlot ();

    void editDataBlockSlot();
    void editCategoriesSlot();
    void refreshjASTERIXSlot();

    void updateSlot();

public:
    ASTERIXConfigWidget(ASTERIXImporterTask& task, QWidget* parent=nullptr);
    virtual ~ASTERIXConfigWidget();

protected:
    ASTERIXImporterTask& task_;
    std::shared_ptr<jASTERIX::jASTERIX> jasterix_;

    QVBoxLayout* main_layout_ {nullptr};
    QGridLayout* categories_grid_ {nullptr};

    ASTERIXFramingComboBox* framing_combo_ {nullptr};
    QPushButton* framing_edit_ {nullptr};

    void updateFraming ();
    void updateCategories ();
};

#endif // ASTERIXCONFIGWIDGET_H
