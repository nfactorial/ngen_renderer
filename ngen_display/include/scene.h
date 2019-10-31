
#define NGEN_NODE_MAXIMUM_NAME_LENGTH       64;

class Scene {
public:
    Scene();
    ~Scene();

    Node* findNodeByName(const char *name);
    Node* findNodeByLocalId(const int id);
    Node* findNodeByNetworkId(const int id);
};

class Entity {
public:

private:
    char    m_name[NGEN_NODE_MAXIMUM_NAME_LENGTH];
};
