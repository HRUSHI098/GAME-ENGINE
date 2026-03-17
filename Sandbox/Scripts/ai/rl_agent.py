"""
rl_agent.py — Reinforcement learning agent hook.

The engine exposes a ge_engine.MLEnvironment for each entity that has an
MLEnvironmentComponent. This script shows how to run a training loop from
Python, or how to drive inference at runtime.

Usage (training, run from Python directly):
  python rl_agent.py

Usage (runtime inference — attach as AIComponent):
  entity.AddComponent<AIComponent>("rl_agent", "RLInferenceAgent")
"""
import ge_engine as ge
import math, random


# ─────────────────────────────────────────────────────────────────────────────
# Inference-time agent (runs inside the engine each frame)
# ─────────────────────────────────────────────────────────────────────────────
class RLInferenceAgent:
    """
    Lightweight inference wrapper.  Loads a trained policy (numpy / pickle)
    and applies it each frame via the ge_engine API.

    Replace _load_policy() with your actual model loader (torch, sklearn, etc.)
    """

    def on_create(self, entity_id: int):
        self._entity_id = entity_id
        self._policy    = self._load_policy()
        ge.Log.info(f"RLInferenceAgent ready on entity {entity_id}")

    def on_update(self, entity_id: int, dt: float):
        obs    = self._get_observation(entity_id)
        action = self._policy(obs)
        self._apply_action(entity_id, action)

    def on_destroy(self, entity_id: int):
        ge.Log.info(f"RLInferenceAgent destroyed on entity {entity_id}")

    # ── Observation: [pos_x, pos_y, vel_x, vel_y] ────────────────────────────
    def _get_observation(self, entity_id: int):
        pos = ge.get_position(entity_id)
        vel = ge.get_velocity(entity_id)
        return [pos.x, pos.y, vel.x, vel.y]

    # ── Action: [move_x, jump] ────────────────────────────────────────────────
    def _apply_action(self, entity_id: int, action):
        vx   = float(action[0]) * 4.0
        jump = float(action[1]) > 0.5
        vel  = ge.get_velocity(entity_id)
        ge.set_velocity(entity_id, ge.Vector2(vx, vel.y))
        if jump and abs(vel.y) < 0.05:
            ge.apply_impulse(entity_id, ge.Vector2(0.0, 6.0))

    def _load_policy(self):
        """Return a callable obs -> action. Replace with real model."""
        # Placeholder: random policy for demonstration
        def random_policy(obs):
            return [random.uniform(-1, 1), random.choice([0.0, 1.0])]
        return random_policy


# ─────────────────────────────────────────────────────────────────────────────
# Offline training loop (run standalone, not inside the engine)
# ─────────────────────────────────────────────────────────────────────────────
def run_training_loop(env_id: int, episodes: int = 100, max_steps: int = 500):
    """
    Example training loop using the ge_engine.MLEnvironment.
    In practice you'd replace the random agent with Q-learning / PPO / etc.
    """
    env = ge.get_ml_environment(env_id)
    if env is None:
        ge.Log.error("No MLEnvironment found on entity — attach MLEnvironmentComponent first.")
        return

    ge.Log.info(f"Training on env '{env.name()}' | obs={env.observation_size()} act={env.action_size()}")

    total_reward = 0.0
    for ep in range(episodes):
        obs  = env.reset()
        done = False
        ep_reward = 0.0
        step = 0

        while not done and step < max_steps:
            # Random agent — replace with your policy
            action = [random.uniform(-1, 1) for _ in range(env.action_size())]
            obs, reward, done = env.step(action)
            ep_reward += reward
            step += 1

        total_reward += ep_reward
        if (ep + 1) % 10 == 0:
            ge.Log.info(f"  Episode {ep+1}/{episodes} avg_reward={total_reward/(ep+1):.3f}")

    ge.Log.info(f"Training done. Mean reward: {total_reward/episodes:.3f}")
