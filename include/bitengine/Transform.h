#pragma once

#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>

namespace BitEngine{


class Transform
{
    public:
        Transform();

        inline void SetPosition(const glm::vec3& p){
            position = p;
            isDirty = true;
        }
        inline void SetScale(const glm::vec3& s){
            scale = s;
            isDirty = true;
        }
        inline void SetRotation(const glm::quat& r){
            rotation = r;
            isDirty = true;
        }

        const glm::vec3& GetPosition() const;
        const glm::vec3& GetScale() const;
        const glm::quat& GetRotation() const;

        const glm::mat4& GetMatrix() const;

        Transform& operator =(const Transform& t);
        const Transform operator *(const Transform& t) const;

        inline bool IsDirty() const {
            return isDirty;
        }
        inline void SetClean() {
            isDirty = false;
        }

    private:
        glm::vec3 position;
        glm::quat rotation;
        glm::vec3 scale;

        mutable glm::mat4 matrix;

        /**
         * If true, recalculates the transformation matrix
         */
        mutable bool isDirty;
};



}
