#include <QString>
#include <QRegularExpression>
#include <QStringList>
#include <QTextStream>
#include <QCryptographicHash>
#include <QMap>
#include <QDirIterator>

#include "lib/godot.h"

namespace godot {
  QMap<QString, QMap<QString, QString>> resourceTemplates = {};
  QMap<QString, QString> nameToResourceTemplateLookup = {};

  void initGodotResourceTemplates() {
    nameToResourceTemplateLookup["paper"] = "concrete";
    nameToResourceTemplateLookup["concrete"] = "concrete";
    nameToResourceTemplateLookup["cardboard"] = "concrete";
    nameToResourceTemplateLookup["cement"] = "concrete";
    nameToResourceTemplateLookup["gravel"] = "concrete";
    nameToResourceTemplateLookup["pavement"] = "concrete";
    nameToResourceTemplateLookup["road"] = "concrete";
    nameToResourceTemplateLookup["asphalt"] = "concrete";
    nameToResourceTemplateLookup["brick"] = "brickwall";
    nameToResourceTemplateLookup["wall"] = "brickwall";
    nameToResourceTemplateLookup["cobblestone"] = "cobblestone";
    nameToResourceTemplateLookup["grass"] = "grass";
    nameToResourceTemplateLookup["sand"] = "grass";
    nameToResourceTemplateLookup["beach"] = "grass";
    nameToResourceTemplateLookup["terrain"] = "grass";
    nameToResourceTemplateLookup["field"] = "grass";
    nameToResourceTemplateLookup["leaves"] = "grass_leafs";
    nameToResourceTemplateLookup["leafs"] = "grass_leafs";
    nameToResourceTemplateLookup["forrest"] = "grass_leafs";
    nameToResourceTemplateLookup["metal_sheet"] = "metal_sheet";
    nameToResourceTemplateLookup["metal"] = "metal_sheet";
    nameToResourceTemplateLookup["porcelain"] = "metal_sheet";
    nameToResourceTemplateLookup["planks"] = "planks";
    nameToResourceTemplateLookup["wood"] = "planks";
    nameToResourceTemplateLookup["walnut"] = "planks";
    nameToResourceTemplateLookup["oak"] = "planks";
    nameToResourceTemplateLookup["bark"] = "planks";
    nameToResourceTemplateLookup["shelf"] = "planks";
    nameToResourceTemplateLookup["plaster"] = "plaster_damaged";
    nameToResourceTemplateLookup["stone"] = "stone";
    nameToResourceTemplateLookup["rock"] = "stone";
    nameToResourceTemplateLookup["tiles"] = "tiles";
    nameToResourceTemplateLookup["tile"] = "tiles";
    nameToResourceTemplateLookup["sofa"] = "fabric";
    nameToResourceTemplateLookup["clothes"] = "fabric";
    nameToResourceTemplateLookup["cloth"] = "fabric";
    nameToResourceTemplateLookup["carpet"] = "fabric";
    nameToResourceTemplateLookup["fabric"] = "fabric";
    nameToResourceTemplateLookup["fur"] = "fabric";
    nameToResourceTemplateLookup["painting"] = "painting";
    nameToResourceTemplateLookup["ground"] = "concrete";
    nameToResourceTemplateLookup["surface"] = "concrete";
    nameToResourceTemplateLookup["marble"] = "marble";
    nameToResourceTemplateLookup["mosaic"] = "marble";
    nameToResourceTemplateLookup["roman"] = "marble";
    nameToResourceTemplateLookup["gold"] = "marble";
    nameToResourceTemplateLookup["glitter"] = "marble";
    nameToResourceTemplateLookup["carbon"] = "marble";

    QDirIterator it(":", QDirIterator::Subdirectories);
    while (it.hasNext()) {
      QString path_resource = it.next();
      if (!path_resource.endsWith(".tres"))
        continue;

      QFileInfo info(path_resource);
      QFile file(path_resource);
      file.open(QIODevice::ReadOnly | QIODevice::Text);
      QTextStream in(&file);
      in.setEncoding(QStringConverter::Utf8);

      QString content = in.readAll();
      resourceTemplates[info.baseName()] = parseResource(content);
    }
  }

  QString generateMaterialTres(QSharedPointer<Texture> tex, TextureSize tsize) {
    const auto diffuse = tex->get_image(TextureImageType::diffuse, tsize);
    if (diffuse.isNull()) {
      qWarning() << "diffuse is null for tex" << tex->name;
      return "";
    }

    const QString asset_pack = tex->asset_pack()->name();
    QString texname = tex->name;

    bool isAlpha = diffuse->isAlpha();
    auto arm = tex->get_image(TextureImageType::arm, tsize);
    auto spec = tex->get_image(TextureImageType::specular, tsize);
    auto metal = tex->get_image(TextureImageType::metalness, tsize);
    auto roughness = tex->get_image(TextureImageType::roughness, tsize);
    auto ao = tex->get_image(TextureImageType::ao, tsize);
    auto displacement = tex->get_image(TextureImageType::displacement, tsize);
    auto normal = tex->get_image(TextureImageType::normal, tsize);
    auto opacity = tex->get_image(TextureImageType::opacity, tsize);
    auto emission = tex->get_image(TextureImageType::emission, tsize);
    auto scattering = tex->get_image(TextureImageType::scattering, tsize);

    struct Resource {
      QSharedPointer<TextureImage> img;
      QString uid;
    };

    QMap<TextureImageType, Resource> resource_ids;

    QString output;
    QTextStream out(&output);

    out << "[gd_resource type=\"StandardMaterial3D\" load_steps=${STEPS} format=3 template=\"${TEMPLATE}\"]\n\n";

    int idx = 1;
    for (const auto &img: tex->textures) {
      auto filename = img->path.fileName();
      auto uid = genID(idx, filename);
      QString header = QString("[ext_resource type=\"Texture2D\" path=\"res://textures/%1/%2\" id=\"%3\"]\n").arg(
        asset_pack).arg(filename).arg(uid);
      out << header;

      resource_ids[img->type] = Resource{.img = img, .uid = uid};
      idx += 1;
    }

    out << "\n[resource]\n";

    // determine what resource template to adopt
    auto name_resource_keys = nameToResourceTemplateLookup.keys();

    QString resourceTemplateID;

    // sad attempt #1
    if (resourceTemplateID.isEmpty()) {
      const auto snake = camelToSnake(tex->name);
      for (const auto no_numbered_snake = removeNumbers(snake);
           const auto& dismembered_snake: no_numbered_snake.split("_")) {
        if (nameToResourceTemplateLookup.contains(dismembered_snake)) {
          resourceTemplateID = nameToResourceTemplateLookup[dismembered_snake];
          break;
        }
      }
    }

    if (resourceTemplateID.isEmpty()) {
      // attempt #1
      for (const auto& tmpl_name: name_resource_keys) {
        if (tex->name_lower.startsWith(tmpl_name)) {
          resourceTemplateID = tmpl_name;
          break;
        }
      }

      // attempt #2
      for (const auto& tmpl_name: name_resource_keys) {
        if (tex->name_lower.contains(tmpl_name)) {
          resourceTemplateID = tmpl_name;
          break;
        }
      }
    }

    // attempt #3
    if (resourceTemplateID.isEmpty()) {
      for (const auto& tag: tex->tags().keys()) {
        if (nameToResourceTemplateLookup.contains(tag)) {
          resourceTemplateID = nameToResourceTemplateLookup[tag];
          break;
        }
      }
    }

    if (resourceTemplateID.isEmpty()) {
      // exhausted, pick default
      resourceTemplateID = "default";
    }

    auto tmpl = resourceTemplates[resourceTemplateID];
    QMap<QString, QString> options;

    // produce .tres body
    options["albedo_texture"] = QString("ExtResource(\"%1\")").arg(resource_ids[TextureImageType::diffuse].uid);

    if (!arm.isNull()) {
      // ao
      options["ao_enabled"] = "true";
      options["ao_light_affect"] = tmpl.contains("ao_light_affect") ? tmpl["ao_light_affect"] : "1.0";
      options["ao_texture"] = QString("ExtResource(\"%1\")").arg(resource_ids[TextureImageType::arm].uid);
      // roughness
      options["roughness_texture_channel"] = "1";
      options["roughness"] = tmpl.contains("roughness") ? tmpl["roughness"] : "0.8";
      options["roughness_texture"] = QString("ExtResource(\"%1\")").arg(resource_ids[TextureImageType::arm].uid);
      // metalness
      options["metallic_texture_channel"] = "2";
      options["metallic"] = tmpl.contains("metallic") ? tmpl["metallic"] : "0.4";
      options["metallic_specular"] = tmpl.contains("metallic_specular") ? tmpl["metallic_specular"] : "0.4";
      options["metallic_texture"] = QString("ExtResource(\"%1\")").arg(resource_ids[TextureImageType::arm].uid);
    }

    if (arm.isNull() && !ao.isNull()) {
      options["ao_enabled"] = "true";
      options["ao_light_affect"] = tmpl.contains("ao_light_affect") ? tmpl["ao_light_affect"] : "1.0";
      options["ao_texture"] = QString("ExtResource(\"%1\")").arg(resource_ids[TextureImageType::ao].uid);
    }

    if (arm.isNull() && !roughness.isNull()) {
      options["roughness"] = tmpl.contains("roughness") ? tmpl["roughness"] : "0.8";
      options["roughness_texture"] = QString("ExtResource(\"%1\")").arg(resource_ids[TextureImageType::roughness].uid);
    }

    if (arm.isNull() && !metal.isNull()) {
      options["metallic"] = tmpl.contains("metallic") ? tmpl["metallic"] : "0.4";
      options["metallic_specular"] = tmpl.contains("metallic_specular") ? tmpl["metallic_specular"] : "0.4";
      options["metallic_texture"] = QString("ExtResource(\"%1\")").arg(resource_ids[TextureImageType::metalness].uid);
    }

    if (!normal.isNull()) {
      options["normal_enabled"] = "true";
      options["normal_scale"] = tmpl.contains("normal_scale") ? tmpl["normal_scale"] : "0.4";
      options["normal_texture"] = QString("ExtResource(\"%1\")").arg(resource_ids[TextureImageType::normal].uid);
    }

    if (arm.isNull() && roughness.isNull()) {
      options["roughness"] = "0.8";
    }

    if (arm.isNull() && metal.isNull() && !spec.isNull()) {
      options["metallic"] = tmpl.contains("metallic") ? tmpl["metallic"] : "0.4";
      options["metallic_specular"] = tmpl.contains("metallic_specular") ? tmpl["metallic_specular"] : "0.2";
      options["metallic_texture"] = QString("ExtResource(\"%1\")").arg(resource_ids[TextureImageType::specular].uid);
    }

    if (!emission.isNull()) {
      options["emission_enabled"] = "true";
      options["emission_texture"] = QString("ExtResource(\"%1\")").arg(resource_ids[TextureImageType::emission].uid);
    }

    if (!scattering.isNull()) {
      options["subsurf_scatter_enabled"] = "true";
      if (texname.contains("skin"))
        options["subsurf_scatter_skin_mode"] = "true";
      options["subsurf_scatter_texture"] = QString("ExtResource(\"%1\")").arg(resource_ids[TextureImageType::scattering].uid);
    }

    if (isAlpha) {
      // alpha from diffuse, A channel
      options["transparency"] = "2";
      options["alpha_scissor_threshold"] = "0.5";
      options["alpha_antialiasing_mode"] = "1";
      options["alpha_antialiasing_edge"] = "0.01";
      options["cull_mode"] = "2";
    }

    for (const auto &[key, value] : options.toStdMap()) {
      auto line = QString("%1 = %2\n").arg(key).arg(value);
      out << line;
    }

    output = output.replace("${STEPS}", QString::number(resource_ids.keys().size() + 1));
    output = output.replace("${TEMPLATE}", resourceTemplateID);

    return output;
  }

  QMap<QString, QString> parseResource(const QString& inp) {
    QMap<QString, QString> options;
    QStringList lines = inp.split('\n', Qt::SkipEmptyParts);

    for (const QString &line : lines) {
      QString trimmed = line.trimmed();
      if (trimmed.startsWith('[') || trimmed.isEmpty()) continue;

      int eqIndex = trimmed.indexOf('=');
      if (eqIndex > 0) {
        QString key = trimmed.left(eqIndex).trimmed();
        QString value = trimmed.mid(eqIndex + 1).trimmed();
        options[key] = value;
      }
    }

    return options;
  }

  QString genID(int idx, const QString &filename) {
    QString uniqueString = QUuid::createUuid().toString(QUuid::WithoutBraces);
    QByteArray hash = QCryptographicHash::hash(filename.toUtf8(), QCryptographicHash::Md5).toHex().left(5);
    return QString::number(idx) + "_" + hash;
  }

  QString camelToSnake(const QString &camel) {
    QString snake = camel;
    QRegularExpression regex("([a-z0-9])([A-Z])");
    snake.replace(regex, "\\1_\\2");
    return snake.toLower();
  }

  QString removeNumbers(QString input) {
    input.remove(QRegularExpression("\\d"));
    return input;
  }
}