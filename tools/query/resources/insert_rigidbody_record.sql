insert into mmq_rigidbodies (
  `index`,
  `parent_model`,
  `name_ja`,
  `name_en`,
  `object_type`,
  `shape_type`,
  `mass`,
  `linear_damping`,
  `angular_damping`,
  `friction`,
  `restitution`
) values (
  :index,
  :parent_model,
  :name_ja,
  :name_en,
  :object_type,
  :shape_type,
  :mass,
  :linear_damping,
  :angular_damping,
  :friction,
  :restitution
)
