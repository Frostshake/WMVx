#pragma once

#include "GameClientInfo.h"
#include <QString>
#include <QFile>
#include <qtcsv/reader.h>
#include <map>

namespace core {

	class BuildInfoFileReader : protected QtCSV::Reader::AbstractProcessor {
	public:
		BuildInfoFileReader(QString fileName) {
			headerFound = false;
			valid = true;

			QFile source(fileName);
			if (source.open(QIODevice::ReadOnly)) {
				QtCSV::Reader::readToProcessor(source, *this, "|");
				source.close();
			}
		}
		virtual ~BuildInfoFileReader() {}

		bool isValid() const {
			return valid;
		}

		std::map<QString, GameClientVersion> getVersions() const {
			std::map<QString, GameClientVersion> versions;

			const auto version_index = fieldIndex("Version!STRING:0");
			const auto product_index = fieldIndex("Product!STRING:0");

			if (version_index >= 0 && product_index >= 0) {
				for (const auto& row : rows) {
					auto ver = GameClientVersion::fromString(row[version_index]);
					if (ver.has_value()) {
						versions.emplace(row[product_index], std::move(*ver));
					}
				}
			}

			return versions;
		}
		

	protected:

		bool processRowElements(const QList<QString>& elements) override {
			if (!headerFound) {
				headerFound = true;

				valid = elements.size() <= MAX_ELEMENTS;

				if (valid) {
					header = elements;
				}
			} else if (rows.size() < MAX_ROWS) {

				valid = elements.size() == header.size();

				if (valid) {
					rows.push_back(elements);
				}
			}
			else {
				valid = false;
			}

			return valid;
		}

		int32_t fieldIndex(QString headerName) const {
			auto it = std::find(header.cbegin(), header.cend(), headerName);

			if (it != header.cend()) {
				return it - header.cbegin();
			}

			return -1;
		}

		QStringList header;
		std::vector<QStringList> rows;

	private:
		bool headerFound;
		bool valid;


		// unsure of expected column / row count, do basic checks to avoid bad data.
		static constexpr int32_t MAX_ELEMENTS = 25;
		static constexpr int32_t MAX_ROWS = 10;
	};

}