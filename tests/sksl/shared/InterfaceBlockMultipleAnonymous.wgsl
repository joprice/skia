### Compilation failed:

error: :10:53 error: unresolved identifier 'x'
    (*_stageOut).sk_FragColor = vec4<f32>(vec2<f32>(x), vec2<f32>(y));
                                                    ^


diagnostic(off, derivative_uniformity);
struct FSIn {
  @builtin(front_facing) sk_Clockwise: bool,
};
struct FSOut {
  @location(0) sk_FragColor: vec4<f32>,
};
fn main(_stageOut: ptr<function, FSOut>) {
  {
    (*_stageOut).sk_FragColor = vec4<f32>(vec2<f32>(x), vec2<f32>(y));
  }
}
@fragment fn fragmentMain(_stageIn: FSIn) -> FSOut {
  var _stageOut: FSOut;
  main(&_stageOut);
  return _stageOut;
}

1 error
