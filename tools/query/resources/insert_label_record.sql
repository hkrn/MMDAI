insert into mmq_labels (
  `index`,
  `parent_model_id`,
  `name_ja`,
  `name_en`,
  `is_special`
) values (
  :index,
  :parent_model,
  :name_ja,
  :name_en,
  :is_special
)
