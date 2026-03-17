"""
patrol_agent.py — Simple patrol AI using the ge_engine Python API.

Attach to an entity via:
  entity.AddComponent<AIComponent>("patrol_agent", "PatrolAgent")
"""
import ge_engine as ge
import math


class PatrolAgent:
    def __init__(self):
        self._direction = 1.0       # 1 = right, -1 = left
        self._patrol_min = -2.0
        self._patrol_max =  2.0
        self._speed      =  2.0
        self._entity_id  =  0

    def on_create(self, entity_id: int):
        self._entity_id = entity_id
        ge.Log.info(f"PatrolAgent created on entity {entity_id}")

    def on_update(self, entity_id: int, dt: float):
        pos = ge.get_position(entity_id)

        # Flip direction at patrol boundaries
        if pos.x >= self._patrol_max:
            self._direction = -1.0
        elif pos.x <= self._patrol_min:
            self._direction = 1.0

        # Move horizontally
        vel = ge.get_velocity(entity_id)
        ge.set_velocity(entity_id, ge.Vector2(self._direction * self._speed, vel.y))

    def on_destroy(self, entity_id: int):
        ge.Log.info(f"PatrolAgent destroyed on entity {entity_id}")

    # ── BT leaf node function ─────────────────────────────────────────────────
    def patrol(self, entity_id: int, dt: float) -> str:
        """Called by BTPyLeaf('patrol') — returns 'success'/'failure'/'running'."""
        self.on_update(entity_id, dt)
        return "running"

    def is_near_boundary(self, entity_id: int, dt: float) -> str:
        pos = ge.get_position(entity_id)
        near = abs(pos.x - self._patrol_max) < 0.3 or abs(pos.x - self._patrol_min) < 0.3
        return "success" if near else "failure"
