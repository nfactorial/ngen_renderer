
constexpr size_t NGEN_NODE_MAXIMUM_NAME_LENGTH = 64;

// TODO: Scene and node definitions should be in their own modules, this file is just for thoughts during development.
class Scene {
public:
    Scene();
    ~Scene();

    void update(const UpdateArgs &args);
    void postUpdate(const UpdateArgs &args);

    Node* findNodeByName(const char *name);
    Node* findNodeByLocalId(const int id);
    Node* findNodeByNetworkId(const int id);

    const std::vector<Node*>& getNodes() const;

private:
    std::vector<Node*>  m_nodes;

    unsigned int m_nextId;
};

const std::vector<Node*>& Scene::getNodes() const {
    return m_nodes;
}

class Node {
public:
    Node();
    ~Node();

    void moveTo(const Vector3 position);
    void translate(const Vector3 distance);

    bool requiresUpdate() const;

    const Vector3 getLocalPosition() const;
    const Vector3 getWorldPosition() const;

    const Vector3 getUp() const;
    const Vector3 getRight() const;
    const Vector3 getForward() const;

    const BoundingBox& getBoundingBox() const;

    const char* getName() const;

    bool hasComponent(const char *name) const;
    IComponent* getComponent(const char *name) const;

private:
    Matrix3x3 m_transform;
    Vector3 m_position;

    char m_name[NGEN_NODE_MAXIMUM_NAME_LENGTH];
    bool m_boundsDirty;

    int m_updateCounter = 0;
    Node *m_parent = nullptr;
};

inline const Matrix3x3& Node::getLocalTransform() const {
    return m_transform;
}

inline bool Node::requiresUpdate() const {
    return 0 != m_updateCounter;
}

inline const char* Node::getName() const {
    return m_name;
}

inline const Vector3 Node::GetUp() const {
    return m_transform[1];
}

inline const Vector3 Node::getRight() const {
    return m_transform[0];
}

inline const Vector3 Node::getForward() const {
    return m_transform[2];
}
