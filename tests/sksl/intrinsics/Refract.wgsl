diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
  @builtin(position) sk_FragCoord: vec4<f32>,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
struct _GlobalUniforms {
  a: f32,
  b: f32,
  c: f32,
  d: vec4<f32>,
  e: vec4<f32>,
};
@binding(0) @group(0) var<uniform> _globalUniforms: _GlobalUniforms;
fn main(_skParam0: vec2<f32>) -> vec4<f32> {
  {
    var result: vec4<f32>;
    let _skTemp0 = refract(vec2<f32>(_globalUniforms.a, 0), vec2<f32>(_globalUniforms.b, 0), _globalUniforms.c).x;
    result.x = _skTemp0;
    let _skTemp1 = refract(_globalUniforms.d, _globalUniforms.e, _globalUniforms.c);
    result = _skTemp1;
    result = vec4<f32>((vec2<f32>(0.5, -0.8660254)), result.zw).xyzw;
    result = vec4<f32>((vec3<f32>(0.5, 0.0, -0.8660254)), result.w).xyzw;
    result = vec4<f32>(0.5, 0.0, 0.0, -0.8660254);
    return result;
  }
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  _stageOut.sk_FragColor = main(_stageIn.sk_FragCoord.xy);
  return _stageOut;
}
