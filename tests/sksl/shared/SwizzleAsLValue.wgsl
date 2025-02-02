diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
  @builtin(position) sk_FragCoord: vec4<f32>,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  colorGreen: vec4<f32>,
  colorRed: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn main(_skParam0: vec2<f32>) -> vec4<f32> {
  {
    var scalar: vec4<f32>;
    var _array: array<vec4<f32>, 1>;
    scalar = vec4<f32>(_globalUniforms.colorGreen) * 0.5;
    scalar.w = 2.0;
    scalar.y = scalar.y * 4.0;
    scalar = vec4<f32>((scalar.yzw * mat3x3<f32>(0.5, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.5)), scalar.x).wxyz;
    scalar = (scalar.zywx + vec4<f32>(0.25, 0.0, 0.0, 0.75)).wyxz;
    scalar.x = scalar.x + (select(0.0, scalar.z, scalar.w <= 1.0));
    _array[0] = vec4<f32>(_globalUniforms.colorGreen) * 0.5;
    _array[0].w = 2.0;
    _array[0].y = _array[0].y * 4.0;
    _array[0] = vec4<f32>((_array[0].yzw * mat3x3<f32>(0.5, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.5)), _array[0].x).wxyz;
    _array[0] = (_array[0].zywx + vec4<f32>(0.25, 0.0, 0.0, 0.75)).wyxz;
    _array[0].x = _array[0].x + (select(0.0, _array[0].z, _array[0].w <= 1.0));
    return select(_globalUniforms.colorRed, _globalUniforms.colorGreen, vec4<bool>(all(scalar == vec4<f32>(1.0, 1.0, 0.25, 1.0)) && all(_array[0] == vec4<f32>(1.0, 1.0, 0.25, 1.0))));
  }
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = main(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
