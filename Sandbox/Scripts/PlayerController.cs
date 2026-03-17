using System;
using GE;

/// <summary>
/// Example C# script — attach via entity.AddComponent<ScriptComponent>("PlayerController").
/// Compiled into SandboxScripts.dll; loaded by ScriptEngine::LoadAppAssembly().
/// </summary>
public class PlayerController : Entity
{
    // Public fields can be exposed to the editor in a later phase.
    public float MoveSpeed = 4.0f;
    public float JumpForce = 6.0f;

    private RigidBody2DComponent _rb = null!;
    private bool _grounded = false;

    public override void OnCreate()
    {
        Log.Info($"PlayerController created on entity {ID}.");
        _rb = GetComponent<RigidBody2DComponent>();
    }

    public override void OnUpdate(float dt)
    {
        float vx = 0f;
        if (Input.IsKeyPressed(KeyCode.Left))  vx = -MoveSpeed;
        if (Input.IsKeyPressed(KeyCode.Right)) vx =  MoveSpeed;

        var vel = _rb.LinearVelocity;
        _grounded = Math.Abs(vel.Y) < 0.05f;

        _rb.LinearVelocity = new Vector2(vx, vel.Y);

        if (Input.IsKeyPressed(KeyCode.Up) && _grounded)
            _rb.ApplyImpulse(new Vector2(0f, JumpForce));
    }

    public override void OnDestroy()
    {
        Log.Info($"PlayerController destroyed on entity {ID}.");
    }
}
