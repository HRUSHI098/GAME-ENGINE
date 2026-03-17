#pragma once

#include "GE/Core/Base.h"
#include <vector>
#include <string>
#include <functional>

namespace GE {

// ─── MLEnvironment ────────────────────────────────────────────────────────────
// Gym-compatible RL environment interface.
//
// C++ code (the "engine side") implements GetObservation, ComputeReward, IsDone.
// Python code (the "agent side") calls Step(action) and receives (obs, reward, done).
//
// Typical training loop (Python):
//   env  = ge.MLEnvironment()
//   obs  = env.reset()
//   while True:
//       action = agent.predict(obs)
//       obs, reward, done, info = env.step(action)
//       if done: obs = env.reset()
class MLEnvironment {
public:
    using Observation = std::vector<f32>;
    using Action      = std::vector<f32>;

    virtual ~MLEnvironment() = default;

    // Reset the environment and return the initial observation
    virtual Observation Reset() = 0;

    // Advance the environment by one step given an action
    // Returns (observation, reward, done)
    virtual std::tuple<Observation, f32, bool> Step(const Action& action) = 0;

    // Space descriptions (for logging / auto-validation)
    virtual u32 GetObservationSize() const = 0;
    virtual u32 GetActionSize()      const = 0;
    virtual const std::string& GetName() const = 0;

    // Optional: called each frame by Scene to let the environment tick
    virtual void OnUpdate(f32 dt) {}
};

// ─── MLEnvironmentComponent ───────────────────────────────────────────────────
// Attach a concrete MLEnvironment to an entity.
// The Python agent retrieves it via ge.get_ml_environment(entity_id).
struct MLEnvironmentComponent {
    Scope<MLEnvironment> Environment = nullptr;

    explicit MLEnvironmentComponent(Scope<MLEnvironment> env)
        : Environment(std::move(env)) {}
};

} // namespace GE
