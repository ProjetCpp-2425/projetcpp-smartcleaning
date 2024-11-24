#include "mainwindow.h"
#include "qsqlerror.h"
#include "ui_mainwindow.h"
#include "housekeeper.h"
#include "QMessageBox"
#include "QSqlQueryModel"
#include "QSqlQuery"
#include "QFileDialog"
#include <QPdfWriter>
#include <QPainter>
#include <QTextDocument>
#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QColor>
#include <QTextCharFormat>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_ajouter_clicked()
{
    QString nomf = ui->nom->text();
    QString prenomf = ui->prenom->text();
    QString adressef = ui->adresse->text();
    QString idf_str = ui->id->text();
    QString telf = ui->tel->text();
    //  QString type = ui->type->text();
    // QString nb_taches = ui->nb_taches->text();


    // Conversion des champs numériques avec vérification
    bool okId; //, okNbTaches = false, okSalaire;
    int idf = idf_str.toInt(&okId);
    //  int salaire = ui->lineEdit_5->text().toInt(&okSalaire);

    // Vérification de l'ID
    if (!okId || idf_str.isEmpty()) {
        QMessageBox::warning(this, QObject::tr("Erreur ID"), QObject::tr("L'ID doit être un entier valide."));
        return;
    }

    // Vérification si l'ID existe déjà
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM HOUSEKEEPER WHERE IDF = :idf");
    query.bindValue(":idf", idf);
    query.exec();

    if (query.next()) {
        int count = query.value(0).toInt();
        if (count > 0) {
            // Mise à jour du housekeeper existant
            Housekeeper H(nomf, prenomf, adressef, idf, telf);
            bool test = H.modifier();
            if (test) {
                QMessageBox::information(this, QObject::tr("Succès"), QObject::tr("Les informations ont été mises à jour avec succès!"));
            } else {
                QMessageBox::critical(this, QObject::tr("Erreur"), QObject::tr("Erreur lors de la mise à jour des informations."));
            }
        } else {
            // Ajout d'un nouveau housekeeper
            Housekeeper H(nomf, prenomf, adressef, idf, telf);
            bool test = H.ajouter();
            if (test) {
                QMessageBox::information(this, QObject::tr("Succès"), QObject::tr("Ajout effectué avec succès!"));
            } else {
                QMessageBox::critical(this, QObject::tr("Erreur"), QObject::tr("Erreur lors de l'ajout du housekeeper."));
            }
        }
    }
}



void MainWindow::on_pushButton_supprimer_clicked()
{
    int idf = ui->lineEditSupprimer->text().toInt();
    Housekeeper Htmp;
    bool test = Htmp.supprimer(idf);
    if (test) {
        QMessageBox::information(nullptr, QObject::tr("Succès"), QObject::tr("Suppression effectuée avec succès!"));
    } else {
        QMessageBox::critical(nullptr, QObject::tr("Erreur"), QObject::tr("Erreur lors de la suppression du housekeeper."));
    }
}



void MainWindow::on_pushButton_afficher_clicked()
{
    QString adresse = ui->lineEditAfficher->text();
    Housekeeper Htmp;
    model = Htmp.afficher(adresse);
    ui->tableView1->setModel(model);

    if (model->rowCount() == 0) {
        QMessageBox::information(this, QObject::tr("Aucun Résultat"), QObject::tr("Aucun housekeeper trouvé pour l'adresse spécifiée."));
    }
}



void MainWindow::on_pushButton_modifier_clicked()
{
    int idf = ui->id->text().toInt();
    QSqlQuery query;
    query.prepare("SELECT * FROM HOUSEKEEPER WHERE IDF = :idf");
    query.bindValue(":idf", idf);

    if (query.exec() && query.next()) {
        ui->id->setText(query.value(0).toString());
        ui->nom->setText(query.value(1).toString());
        ui->prenom->setText(query.value(2).toString());
        ui->adresse->setText(query.value(3).toString());
        ui->tel->setText(query.value(4).toString());
    } else {
        QMessageBox::warning(this, QObject::tr("Erreur"), QObject::tr("ID introuvable dans la base de données."));
    }
}



void MainWindow::on_pushButton_rechercher_clicked()
{
    bool ok;
    int idf = ui->lineEditSupprimer->text().toInt(&ok);  // Vérifier que la saisie est un entier valide

    if (!ok) {
        QMessageBox::warning(this, QObject::tr("Erreur de saisie"), QObject::tr("Veuillez entrer un ID valide (entier)."));
        return;
    }

    Housekeeper Htmp;
    model = Htmp.rechercher(idf);
    ui->tableView1->setModel(model);

    if (model->rowCount() == 0) {
        QMessageBox::information(this, QObject::tr("Aucun Résultat"), QObject::tr("Aucun housekeeper trouvé pour l'ID spécifié."));
    }
}




void MainWindow::on_pushButton_trier_clicked()
{
    Housekeeper Htmp;
    model = Htmp.trier();  // Appeler la méthode trier()
    ui->tableView1->setModel(model);

    if (model->rowCount() == 0) {
        QMessageBox::information(this, QObject::tr("Aucun Résultat"), QObject::tr("Aucun housekeeper trouvé."));
    }
}



void MainWindow::on_pushButton_pdf_clicked()
{
    QFile pdfFile("D:/Oracle/Atelier_Connexion/housekeepers_list.pdf");
    if (pdfFile.open(QIODevice::WriteOnly)) {
        QPdfWriter writer(&pdfFile);
        writer.setPageSize(QPageSize::A4);
        QPainter painter(&writer);
        QFont font = painter.font();
        font.setPointSize(200);
        painter.setFont(font);

        QTextDocument doc;
        QString content = "<h1 style='font-size: 50pt;'>Housekeepers List</h1>"
                          "<table border='1' cellpadding='5' cellspacing='0' style='font-size: 25pt;'>"
                          "<tr><th>ID</th><th>Last Name</th><th>Name</th><th>Address</th><th>Phone number</th></tr>";
        QSqlQuery query("SELECT IDF, NOMF, PRENOMF, ADRESSEF,TELF FROM HOUSEKEEPER");
        while (query.next()) {
            content += "<tr>"
                       "<td>" + query.value("IDF").toString() + "</td>"
                                                         "<td>" + query.value("NOMF").toString() + "</td>"
                                                          "<td>" + query.value("PRENOMF").toString() + "</td>"
                                                             "<td>" + query.value("ADRESSEF").toString() + "</td>"
                                                              "<td>" + query.value("TELF").toString() + "</td>"
                                                          "</tr>";
        }

        content += "</table>";
        doc.setHtml(content);
        doc.drawContents(&painter);
        painter.end();
    }
    QMessageBox::information(this, "PDF généré", "Le fichier PDF des housekeepers a été mis à jour avec succès.");
}




void MainWindow::on_pushButton_pdf_2_clicked()
{
    QFile pdfFile("D:/Oracle/Atelier_Connexion/housekeepers_list.pdf");
    if (pdfFile.open(QIODevice::WriteOnly)) {
        QPdfWriter writer(&pdfFile);
        writer.setPageSize(QPageSize::A4);
        QPainter painter(&writer);
        QFont font = painter.font();
        font.setPointSize(200);
        painter.setFont(font);

        QTextDocument doc;
        QString content = "<h1 style='font-size: 50pt;'>Housekeepers List</h1>"
                          "<table border='1' cellpadding='5' cellspacing='0' style='font-size: 25pt;'>"
                          "<tr><th>ID</th><th>Last Name</th><th>Name</th><th>Address</th><th>Phone number</th></tr>";
        QSqlQuery query("SELECT IDF, NOMF, PRENOMF, ADRESSEF, TELF FROM HOUSEKEEPER");
        while (query.next()) {
            content += "<tr>"
                       "<td>" + query.value("IDF").toString() + "</td>"
                                                         "<td>" + query.value("NOMF").toString() + "</td>"
                                                          "<td>" + query.value("PRENOMF").toString() + "</td>"
                                                             "<td>" + query.value("ADRESSEF").toString() + "</td>"
                                                              "<td>" + query.value("TELF").toString() + "</td>"
                                                          "</tr>";
        }

        content += "</table>";
        doc.setHtml(content);
        doc.drawContents(&painter);
        painter.end();
    }
    QMessageBox::information(this, "PDF généré", "Le fichier PDF des housekeepers a été mis à jour avec succès.");
}



void MainWindow::on_pushButton_stat_clicked() {
    QGraphicsScene *scene = new QGraphicsScene(this);
    ui->graphic->setScene(scene);

    // Nouvelle requête pour obtenir les statistiques selon la localisation
    QString queryStr = "SELECT adressef, COUNT(*) AS frequency "
                       "FROM HOUSEKEEPER "
                       "GROUP BY adressef "
                       "ORDER BY frequency DESC";

    QSqlQuery query;
    if (!query.exec(queryStr)) {
        QMessageBox::critical(this, "Erreur", "Échec de l'exécution de la requête : " + query.lastError().text());
        return;
    }

    int barWidth = 60;
    int spacing = 25;
    int x = 100;
    int maxBarHeight = 300;
    QFont font("Helvetica", 14, QFont::Bold);
    int maxFrequency = 1;

    // Calculer le maximum de la fréquence
    while (query.next()) {
        int frequency = query.value("frequency").toInt();
        if (frequency > maxFrequency) {
            maxFrequency = frequency;
        }
    }

    query.first(); // Réinitialiser le curseur pour lire les données à nouveau

    int yAxisHeight = maxBarHeight + 50; // Hauteur de l'axe Y
    int xAxisLength = query.size() * (barWidth + spacing); // Longueur de l'axe X

    // Dessiner les axes
    scene->addLine(100, 0, 100, -yAxisHeight, QPen(Qt::black));
    scene->addLine(100, 0, 800 + xAxisLength, 0, QPen(Qt::black));

    QGraphicsTextItem *yAxisLabel = scene->addText("Fréquence", font);
    yAxisLabel->setPos(10, -yAxisHeight - 30);

    QGraphicsTextItem *xAxisLabel = scene->addText("Localisation", font);
    xAxisLabel->setPos(50 + xAxisLength / 2, 30);

    // Dessiner les barres du graphique
    do {
        QString adresse = query.value("adressef").toString();
        int frequency = query.value("frequency").toInt();

        int barHeight = (frequency * maxBarHeight) / maxFrequency;

        // Couleur de la barre par défaut
        QColor barColor = Qt::blue; // Changez la couleur si nécessaire

        QGraphicsRectItem *bar = scene->addRect(x, -barHeight, barWidth, barHeight, QPen(Qt::black), QBrush(barColor));
        bar->setPos(x + 2, 0);

        QGraphicsTextItem *textItem = scene->addText(adresse, font);
        textItem->setPos(x * 1.75 + barWidth - 10, 0);

        QGraphicsTextItem *countText = scene->addText(QString::number(frequency), font);
        countText->setPos(x * 2 + barWidth / 4, -barHeight - 40);

        x += barWidth + spacing;
    } while (query.next());

    ui->graphic->setRenderHint(QPainter::Antialiasing);
    ui->graphic->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);

    qDebug() << "Graphique des statistiques généré avec succès.";
}



void MainWindow::on_pushButton_calculer_clicked() {
    // Récupération des données de l'interface utilisateur
    QString type = ui->comboBox1->currentText(); // Exemple d'un comboBox pour sélectionner le type
    QString nb_taches = ui->comboBox2->currentText();     // Exemple d'un spinBox pour le nombre de tâches

    // Création d'une instance de Housekeeper
    Housekeeper housekeeper;

    // Calcul du salaire
    int salaire = housekeeper.calculerSalaire(type, nb_taches);

    // Affichage du résultat dans l'interface
    ui->label_afficher_salaire->setText(QString::number(salaire) + " DT");
}
