
#include "Transform.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace BitEngine{

Transform::Transform()
    : position(0.0), rotation(), scale(1.0), isDirty(true)
{
}

Transform& Transform::operator =(const Transform& t)
{
    position = t.position;
    rotation = t.rotation;
    scale = t.scale;

    return *this;
}

const Transform Transform::operator *(const Transform& t) const
{
    Transform transform;
    transform.position = position + t.position;
    transform.rotation = rotation * t.rotation;
    transform.scale = scale * t.scale;
    transform.isDirty = true;
    return transform;

}

const glm::mat4& Transform::GetMatrix() const
{
    if (isDirty)
    {
        matrix = glm::translate(glm::mat4(), position );
        matrix *= glm::toMat4(rotation);
        matrix = glm::scale(matrix, scale );
        isDirty = false;
    }

    return matrix;
}


const glm::vec3& Transform::GetPosition() const {
    return position;
}

const glm::vec3& Transform::GetScale() const {
    return scale;
}

const glm::quat& Transform::GetRotation() const {
    return rotation;
}


}
