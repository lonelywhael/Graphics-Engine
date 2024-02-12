#include "gui/model.hpp"

Model::Model(Serializer& object) {
    vertexArray = std::make_shared<VertexArray>(static_cast<Serializer&>(object["vertex_array"]));
    textureGroup = (object["textureGroup"] != nullptr) ? 
                            std::make_shared<TextureGroup>(static_cast<Serializer&>(object["texture_group"])) : 
                            nullptr;
    material = (object["material"] != nullptr) ? std::make_shared<Material>(static_cast<Serializer&>(object["material"])) : nullptr;
    pos = object["pos"];
    scale = object["scale"];
    aos = object["aos"];
    angle = object["angle"];
    color = object["color"];
}
void Model::setModel() {
    // applies transformations to model matrix
    model = glm::mat4(1.0f);                // resets model to identity
    model = glm::translate(model, pos);     // translates in 3D space to be centered on pos vector
    model = glm::scale(model, scale);       // scales in x dir by scale.x, y dir by scale.y, z dir by scale.z
    model = glm::rotate(model, angle, aos); // rotates by angle around the vector aos (axis of symmetry)
}


void Model::generateMaterial(const glm::vec3 specular, const float shininess) {
    // based on the textures present in the texture group, we can generate a material by slotting those textures
    switch(textureGroup->size()) {
    // 1 texture is a D Map
    case 1: { material = std::make_shared<Material>(textureGroup->getSlot(DIFFUSE), specular, shininess ); } break;
    // 2 textures is a DS Map
    case 2: { material = std::make_shared<Material>(textureGroup->getSlot(DIFFUSE), 
                                                    textureGroup->getSlot(SPECULAR), shininess ); } break;
    // 3 textures is a DSE Map
    case 3: { material = std::make_shared<Material>(textureGroup->getSlot(DIFFUSE), 
                                                    textureGroup->getSlot(SPECULAR), 
                                                    textureGroup->getSlot(EMISSION), shininess ); } break;
    }
    // change the model type now that we have lighting information
    type = R_LIGHTING_3D;
}
const glm::vec3 DEFAULT_SPECULAR(0.2f);
const float DEFAULT_SHININESS = 16;
void Model::generateMaterial(const float shininess) { generateMaterial(DEFAULT_SPECULAR, (shininess > 0.0f) ? shininess : DEFAULT_SHININESS); }

Serializer Model::getJSON() {
    Serializer object;
    object["vertex_array"] = std::move(vertexArray->getJSON());
    (textureGroup != nullptr) ? object["texture_group"] = std::move(textureGroup->getJSON()) : object["textureGroup"] = nullptr;
    (material != nullptr) ? object["material"] = std::move(material->getJSON()) : object["material"] = nullptr;
    object["pos"] = pos;
    object["scale"] = scale;
    object["aos"] = aos;
    object["angle"] = angle;
    object["color"] = color;

    return object;
}

void Model::print() const {
    std::cout << "Vertex Array: " << vertexArray << std::endl;
    //mesh->print();
    std::cout << "Pos: (" << pos.x << ", " << pos.y << ", " << pos.z << ")" << std::endl;
    std::cout << "Scale: (" << scale.x << ", " << scale.y << ", " << scale.z << ")" << std::endl;
    std::cout << "Axis of Symmetry: (" << aos.x << ", " << aos.y << ", " << aos.z << ")" << std::endl;
    std::cout << "Angle: " << angle << std::endl;
    std::cout << std::endl; 
}