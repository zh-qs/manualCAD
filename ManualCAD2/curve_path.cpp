#include "curve_path.h"
#include "object.h"
#include "polynomial_algorithms.h"

namespace ManualCAD
{
    std::vector<std::vector<Vector3>> CurvePath::generate_paths() const
    {
        std::vector<std::vector<Vector3>> result;
        result.reserve(beziers.size());

        for (const auto& bezier : beziers)
        {
            if constexpr (ApplicationSettings::DEBUG)
                assert(bezier.size() % 3 == 1);

            size_t curves = bezier.size() / 3;
            const int steps = 20;
            std::vector<Vector3> curve;
            curve.reserve(steps * curves + 1);
            const float step = 1.0f / steps;
            for (int i = 0; i < curves; ++i)
            {
                for (int t = 0; t < steps; ++t)
                    curve.push_back(PolynomialAlgorithms::de_casteljau(bezier[3 * i], bezier[3 * i + 1], bezier[3 * i + 2], bezier[3 * i + 3], t * step));
            }
            curve.push_back(bezier.back());
            result.push_back(curve);
        }

        return result;
    }

    void CurvePath::project_curves(float y)
    {
        for (auto& bezier : beziers)
            for (auto& p : bezier)
                p.y = y;
    }
}
